#!/usr/bin/env Rscript
options("width"=160)
sink(stdout(), type = "message")
library(plyr)
library(ggplot2)

CRYSTAL_TYPE_DATA = 0x02

CRYSTAL_BAD_DATA = 1
CRYSTAL_BAD_CRC = 2
CRYSTAL_HIGH_NOISE = 3
CRYSTAL_SILENCE = 4

ssum = read.table("send_summary.log", header=T, colClasses=c("numeric"))
rsum = read.table("recv_summary.log", header=T, colClasses=c("numeric"))
recv = read.table("recv.log", header=T, colClasses=c("numeric"))
send = read.table("send.log", header=T, colClasses=c("numeric"))
energy = read.table("energy.log", header=T, colClasses=c("numeric"))
energy_tf = read.table("energy_tf.log", header=T, colClasses=c("numeric"))
energy = merge(energy, energy_tf, by=c("epoch", "node"))

params = read.table("params_tbl.txt", header=T)
SINK = params$sink[1]
message("Sink node: ", SINK)

message("Starting epoch per node")
start_epoch = setNames(aggregate(epoch ~ dst, rsum, FUN=min), c("node", "epoch"))
start_epoch

message("Distribution of starting epoch")
setNames(aggregate(node ~ epoch, start_epoch, FUN=length), c("epoch", "n_nodes"))

message("Max starting epoch: ", max(start_epoch$epoch))

SKIP_END_EPOCHS = 30
#SKIP_END_EPOCHS = 1
message("SKIPPING last ", SKIP_END_EPOCHS, " epochs!")

min_epoch = max(c(params$start_epoch, min(rsum$epoch))+1)
max_epoch = max(rsum$epoch)-SKIP_END_EPOCHS
#max_epoch = min(params$start_epoch+params$active_epochs, max(rsum$epoch)-1)


n_epochs = max_epoch - min_epoch + 1
message(n_epochs, " epochs: [", min_epoch, ", ", max_epoch, "]")


e = recv$epoch
recv = recv[e>=min_epoch & e<=max_epoch,]
e = send$epoch
send = send[e>=min_epoch & e<=max_epoch,]
e = ssum$epoch
ssum = ssum[e>=min_epoch & e<=max_epoch,]
e = rsum$epoch
rsum = rsum[e>=min_epoch & e<=max_epoch,]
e = energy$epoch
energy = energy[e>=min_epoch & e<=max_epoch,]
rm(e)

alive = unique(read.table("node.log", header=T, colClasses=c("numeric", "character", "numeric"))$id)
message("Alive nodes: ", length(alive))
print(sort(alive))

heard_from = unique(rsum$dst)
message("Participating nodes: ", length(heard_from))
print(sort(heard_from))

sent_anything = unique(send$src)
message("Nodes that logged sending something: ", length(sent_anything))
print(sort(sent_anything))

not_packets = (recv$type != CRYSTAL_TYPE_DATA) | (recv$err_code != 0)
wrong_packets = not_packets & (recv$err_code %in% c(CRYSTAL_BAD_DATA, CRYSTAL_BAD_CRC))

recv_from = unique(recv[!not_packets, c("src")])
message("Messages received from the following ", length(recv_from), " nodes")
print(sort(recv_from))

message("Alive nodes that didn't work")
lazy_nodes = setdiff(alive, c(heard_from, recv_from, sent_anything, SINK))
lazy_nodes

message("Packets of wrong types or length received (removing them):")
recv[wrong_packets,]

no_packet_ts = recv[recv$dst==SINK & not_packets & (recv$err_code %in% c(CRYSTAL_HIGH_NOISE, CRYSTAL_SILENCE)), c("epoch", "dst", "n_ta", "length", "err_code", "time")]
write.table(no_packet_ts, "empty_t.log", row.names=F)

recv=recv[!not_packets,]

sink_recv = recv[recv$dst==SINK,]
duplicates = duplicated(sink_recv[,c("src", "seqn")])
message("Duplicates: ", sum(duplicates))

sink_rsum = rsum[rsum$dst==SINK,]

message("TX and RX records")
s = setNames(aggregate(epoch ~ ssum$src, ssum, FUN=length), c("node", "count_tx"))
r = setNames(aggregate(epoch ~ rsum$dst, rsum, FUN=length), c("node", "count_rx"))
print(merge(s,r,all=T)); rm(s); rm(r);

message("Distribution of number of consecutive misses of the sync packets from the sink")
print(setNames(aggregate(sync_missed ~ ssum$sync_missed, ssum, length), c("num_misses", "num_records")))

message("Nodes that lost a sync message more than twice in a row")
sort(unique(ssum[ssum$sync_missed>2,c("src")]))

    maxta = setNames(aggregate(
          rsum$n_ta, 
          list(epoch=rsum$epoch), 
            FUN=max),
          c("epoch", "n_ta_max"))

    minta = setNames(aggregate(
          rsum$n_ta, 
          list(epoch=rsum$epoch), 
            FUN=min),
          c("epoch", "n_ta_min"))

    avg_rxta = setNames(aggregate(
          sink_recv$n_ta, 
          list(epoch=sink_recv$epoch), 
            FUN=mean),
          c("epoch", "avg_rxta"))

    avg_rxta$avg_rxta <- avg_rxta$avg_rxta + 1

    deliv = merge(
                merge(
                    merge(
                       minta,
                       maxta, all=T),
                    setNames(sink_rsum[,c("epoch", "n_ta")], c("epoch", "n_ta_sink")), all=T),
                avg_rxta, all=T)

    print(deliv[,c("epoch", "n_ta_sink", "n_ta_min", "n_ta_max", "avg_rxta")])

energy_pernode = setNames(aggregate(ontime ~ energy$node, energy, mean), c("node", "ontime"))

message("Energy summary")
esum = summary(energy_pernode$ontime)
esum

min_ontime = as.numeric(esum[1])
mean_ontime = as.numeric(esum[4])
max_ontime = as.numeric(esum[6])

message("Avg. radio ON time (ms/s): ", mean_ontime)

energy = within(energy, dst<-node)
en = merge(rsum, energy, by=c("epoch","dst"))
en = rename(en, c("dst"="src"))
en = merge(en, ssum[c("epoch", "src", "sync_missed", "n_acks")], by=c("epoch", "src"), all.x=T) 
en = within(en, ton_total<-(ton_s+ton_t+ton_a)/32768*1000)

en = within(en, ton_s <- (ton_s/32768)*1000)
ton_s_pernode = setNames(aggregate(ton_s ~ en$node, en, mean), c("node", "ton_s"))
en_tas = en[en$n_ta>0,] # filtering out zero cases
en_tas = within(en_tas, ton_t <- (ton_t/32768)*1000/n_ta)
en_tas = within(en_tas, ton_a <- (ton_a/32768)*1000/n_ta)
ton_t_pernode = setNames(aggregate(ton_t ~ en_tas$node, en_tas, mean), c("node", "ton_t"))
ton_a_pernode = setNames(aggregate(ton_a ~ en_tas$node, en_tas, mean), c("node", "ton_a"))

energy_pernode = merge(energy_pernode, ton_s_pernode, all.x=T)
energy_pernode = merge(energy_pernode, ton_a_pernode, all.x=T)
energy_pernode = merge(energy_pernode, ton_t_pernode, all.x=T)

ton_total_pernode = setNames(aggregate(ton_total ~ en$node, en, mean), c("node", "ton_total"))
ton_total_mean = mean(en$ton_total)
energy_pernode = merge(energy_pernode, ton_total_pernode, all.x=T)

en = within(en, tf_s <- (tf_s/32768)*1000)
en = within(en, tf_t <- (tf_t/32768)*1000)
en = within(en, tf_a <- (tf_a/32768)*1000)

en_short_s = en[en$n_short_s != 0,]
en_short_t = en[en$n_short_t != 0,]
en_short_a = en[en$n_short_a != 0,]

en_short_t = within(en_short_t, tf_t <- tf_t/n_short_t)
en_short_a = within(en_short_a, tf_a <- tf_a/n_short_a)
en_short_a = within(en_short_a, n_compl_recv_a <- n_short_a/n_ta)

tf_s_pernode = setNames(aggregate(tf_s ~ en_short_s$node, en_short_s, mean), c("node", "tf_s"))
tf_t_pernode = setNames(tryCatch(aggregate(tf_t ~ en_short_t$node, en_short_t, mean), error=function(e) data.frame(matrix(vector(), 0, 2))), c("node", "tf_t"))
tf_a_pernode = setNames(tryCatch(aggregate(tf_a ~ en_short_a$node, en_short_a, mean), error=function(e) data.frame(matrix(vector(), 0, 2))), c("node", "tf_a"))


pdf("tf.pdf", width=12, height=8)

short_s_pernode = setNames(aggregate(s_rx_cnt>=params$n_tx_s ~ rsum$dst, rsum, sum), c("node", "n_short_s"))
n_recv_pernode =  setNames(aggregate(s_rx_cnt>0 ~ rsum$dst, rsum, sum), c("node", "n_recv_s"))
short_s_pernode = merge(short_s_pernode, n_recv_pernode)
short_s_pernode = within(short_s_pernode, n_compl_recv_s <- n_short_s/n_recv_s)
short_s_pernode[!is.finite(short_s_pernode$n_compl_recv_s), c("n_compl_recv_s")] <- NA 

ggplot(data=en_short_s, aes(x=as.factor(node), y=tf_s)) + 
    geom_boxplot(outlier.colour="gray30", outlier.size=1) +
    #xlim(0, max_cca_busy) +    
    ggtitle("tf_s") +
    theme_bw()
ggplot(data=short_s_pernode, aes(x=as.factor(node), y=n_compl_recv_s)) + 
    geom_point() +
    ggtitle("Ratio of S phases with complete receive") +
    theme_bw()

ggplot(data=en_short_a, aes(x=as.factor(node), y=tf_a)) + 
    geom_boxplot(outlier.colour="gray30", outlier.size=1) +
    ggtitle("tf_a (epoch averages)") +
    theme_bw()
ggplot(data=en_short_a, aes(x=as.factor(node), y=n_compl_recv_a)) + 
    geom_boxplot(outlier.colour="gray30", outlier.size=1) +
    ggtitle("Ratio of complete A slots (epoch averages)") +
    theme_bw()
dev.off()

energy_pernode = merge(energy_pernode, tf_s_pernode, all.x=T)
energy_pernode = merge(energy_pernode, tf_a_pernode, all.x=T)
energy_pernode = merge(energy_pernode, tf_t_pernode, all.x=T)

t_compl_recv = sum(recv$rx_cnt>=params$n_tx_t)/dim(recv)[1]
message("T complete receives: ", t_compl_recv)

message("Radio ON per node")
energy_pernode

write.table(energy_pernode, "pernode_energy.txt", row.names=F)


# -- Per-node and advanced stats ------------------------------------------------------

message("Computing the advanced stats")
message("Considering the following senders")
#senders = unique(c(recv_from, sent_anything ))
senders = heard_from[heard_from != SINK]
print(sort(senders))

pernode = data.frame(node=sort(unique(c(recv_from, sent_anything, heard_from))))

# per-node data PDR

pernode_sent = setNames(tryCatch(aggregate(seqn ~ sendrecv$src, sendrecv, FUN=length), error = function(e) data.frame(matrix(vector(), 0, 2))), c("node", "n_sent"))
pernode_recv = setNames(tryCatch(aggregate(seqn ~ sendrecv$src, sendrecv, subset=!is.na(sendrecv$epoch.rx), FUN=length), error = function(e) data.frame(matrix(vector(), 0, 2))), c("node", "n_recv"))
pernode_sent = pernode_sent[pernode_sent$n_sent>0,]

pernode_pdr = within(
                     merge(pernode_sent, pernode_recv),
                     pdr<-n_recv/n_sent)

pernode = merge(pernode, pernode_pdr, all.x=T)

# average number of tx attempts

pernode_ntx = setNames(tryCatch(aggregate(n_tx ~ real_send$src, real_send, FUN=sum), error = function(e) data.frame(matrix(vector(), 0, 2))), c("node", "n_tx_tries"))

pernode = merge(pernode, pernode_ntx, all.x=T)

# counting numbers of lost and received beacons

missed = setNames(tryCatch(aggregate(epoch ~ ssum$src, ssum, subset=ssum$sync_missed>0, FUN=length), error = function(e) data.frame(matrix(vector(), 0, 2))),
                  c("node", "s_missed"))

received = setNames(aggregate(epoch ~ rsum$dst, rsum, subset=rsum$s_rx_cnt>0, FUN=length),
                  c("node", "s_received"))

pernode1 = merge(received, missed, all=T)
pernode1[is.na(pernode1$s_received),c("s_received")] <- 0
pernode1[is.na(pernode1$s_missed),c("s_missed")] <- 0

pernode1 = within(pernode1, s_pdr<-s_received/(s_missed+s_received))
pernode1 = within(pernode1, s_missed_total<-n_epochs-s_received)
pernode1 = within(pernode1, s_pdr_total<-s_received/n_epochs)

total_s_received = sum(pernode1$s_received)

mean_s_pdr = total_s_received/(sum(pernode1$s_missed)+total_s_received)
min_s_pdr = min(pernode1$s_pdr)
max_s_pdr = max(pernode1$s_pdr)
mean_s_pdr_total = total_s_received/((length(senders)+1)*n_epochs) # +1 because the sink is also counted
min_s_pdr_total = min(pernode1$s_pdr_total)
max_s_pdr_total = max(pernode1$s_pdr_total)

pernode = merge(pernode, pernode1, all.x=T)

message("Synch phase reliability stats")
message("S_pdr: ", min_s_pdr, " ", mean_s_pdr, " ", max_s_pdr)
message("s_pdr_total: ", min_s_pdr_total, " ", mean_s_pdr_total, " ", max_s_pdr_total)

pernode_rx_cnt = setNames(aggregate(s_rx_cnt ~ rsum$dst, rsum, subset=rsum$s_rx_cnt>0, FUN=mean), c("node", "s_rx_cnt"))
pernode = merge(pernode, pernode_rx_cnt, all.x=T)

s_compl_recv = sum(rsum$s_rx_cnt>=params$n_tx_s)/(total_s_received)
message("S complete receives: ", s_compl_recv)

t = rsum[rsum$dst!=SINK, c("epoch", "dst", "n_ta")]
t = rename(t, c("dst"="src"))
t = merge(t, ssum[c("epoch", "src", "n_acks")])

tas_pernode = setNames(aggregate(n_ta ~ t$src, t, FUN=sum), c("node", "tas"))
acks_pernode = setNames(aggregate(n_acks ~ t$src, t, FUN=sum), c("node", "acks"))

apdr_pernode = merge(tas_pernode, acks_pernode)
apdr_pernode = within(apdr_pernode, a_pdr<-acks/tas)
pernode = merge(pernode, apdr_pernode[c("node", "a_pdr")], all.x=T)

min_a_pdr = min(apdr_pernode$a_pdr)
max_a_pdr = max(apdr_pernode$a_pdr)
mean_a_pdr = sum(apdr_pernode$acks)/sum(apdr_pernode$tas)
message("A_pdr: ", min_a_pdr, " ", mean_a_pdr, " ", max_a_pdr)

rm(t)
rm(tas_pernode)
rm(acks_pernode)
rm(apdr_pernode)




# nodes that go to sleep earlier or later than the sink

pernode1 = data.frame(node=senders, n_early_sleep=c(0), n_late_sleep=c(0))
n_early = c()
n_late  = c()
for (e in min_epoch:max_epoch) {
    cur = rsum[rsum$epoch==e,]
    tas_diff = cur$n_ta - cur[cur$dst==SINK,]$n_ta
    if (length(tas_diff) == 0) {
      next
    }
    diff_tbl = data.frame(node=cur$dst, tas_diff=tas_diff)
    
    early_nodes = diff_tbl[diff_tbl$tas_diff<0,]$node
    late_nodes  = diff_tbl[diff_tbl$tas_diff>0,]$node
    n_early = c(n_early, length(early_nodes))
    n_late  = c(n_late, length(late_nodes))
    subs_early = pernode1$node %in% early_nodes
    subs_late = pernode1$node %in% late_nodes
    pernode1[subs_early,]$n_early_sleep = pernode1[subs_early,]$n_early_sleep + 1
    pernode1[subs_late, ]$n_late_sleep  = pernode1[subs_late, ]$n_late_sleep  + 1
}
min_early_sleepers  = min(n_early)
mean_early_sleepers = mean(n_early)
max_early_sleepers  = max(n_early)


min_late_sleepers  = min(n_late)
mean_late_sleepers = mean(n_late)
max_late_sleepers  = max(n_late)

message("Min, mean, max # early sleepers: ", min_early_sleepers, " ", mean_early_sleepers, " ", max_early_sleepers)
message("Min, mean, max # late sleepers:  ", min_late_sleepers, " ", mean_late_sleepers, " ", max_late_sleepers)


pernode1 = within(pernode1, early_sl<-n_early_sleep/n_epochs)
pernode1 = within(pernode1, late_sl<-n_late_sleep/n_epochs)
pernode = merge(pernode, pernode1[c("node", "early_sl", "late_sl")], all.x=T)

mean_early_sleep_epochs = mean(pernode$early_sl, na.rm=T)
max_early_sleep_epochs = max(pernode$early_sl, na.rm=T)
mean_late_sleep_epochs = mean(pernode$late_sl, na.rm=T)
max_late_sleep_epochs = max(pernode$late_sl, na.rm=T)
message("Mean and max of early sleep epochs: ", mean_early_sleep_epochs, " ", max_early_sleep_epochs)
message("Mean and max of late sleep epochs: ", mean_late_sleep_epochs, " ", max_late_sleep_epochs)

send_sync = ssum[ssum$sync_missed==0,]
pernode_hops = setNames(aggregate(hops ~ send_sync$src, send_sync, FUN=mean), c("node", "hops"))
pernode_hops$hops <- pernode_hops$hops + 1 # the logged value is actually the "relay count", i.e. hops-1

pernode = merge(pernode, pernode_hops, all.x=T)

hops_mean = mean(send_sync$hops) + 1
hops_max = max(pernode_hops$hops)
message("Mean hops: ", hops_mean)
message("Max hops: ", hops_max)


skew_mean = mean(send_sync$skew)


pernode_skew_min = setNames(aggregate(skew ~ send_sync$src, send_sync, FUN=min), c("node", "skew_min"))
pernode = merge(pernode, pernode_skew_min, all.x=T)
pernode_skew_mean = setNames(aggregate(skew ~ send_sync$src, send_sync, FUN=mean), c("node", "skew_mean"))
pernode = merge(pernode, pernode_skew_mean, all.x=T)
pernode_skew_max = setNames(aggregate(skew ~ send_sync$src, send_sync, FUN=max), c("node", "skew_max"))
pernode = merge(pernode, pernode_skew_max, all.x=T)

skew_diff_all = within(
                       merge(
                             setNames(send_sync[c("src", "skew")], c("node", "skew")), 
                             pernode_skew_mean, all.x=T), 
                       skew_diff<-skew_mean-skew)

pernode_skew_var = setNames(aggregate(abs(skew_diff) ~ skew_diff_all$node, skew_diff_all, FUN=mean), c("node", "skew_var"))
pernode = merge(pernode, pernode_skew_var, all.x=T)

skew_var_mean = mean(pernode_skew_var$skew_var)

pernode_noise_min = setNames(aggregate(noise ~ rsum$dst, rsum, FUN=min), c("node", "noise_min"))
pernode_noise_mean = setNames(aggregate(noise ~ rsum$dst, rsum, FUN=mean), c("node", "noise_mean"))
pernode_noise_max = setNames(aggregate(noise ~ rsum$dst, rsum, FUN=max), c("node", "noise_max"))
pernode = merge(pernode, pernode_noise_min, all.x=T)
pernode = merge(pernode, pernode_noise_mean, all.x=T)
pernode = merge(pernode, pernode_noise_max, all.x=T)
noise_mean = mean(rsum$noise)
noise_max = max(pernode_noise_mean$noise_mean)
message("Mean noise: ", noise_mean)
message("Max noise: ", noise_max)

write.table(pernode, "pernode.txt", row.names=F)


max_cca_busy = max(rsum$cca_busy)

