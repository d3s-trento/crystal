#!/usr/bin/env Rscript
options("width"=160)
sink(stdout(), type = "message")
library(plyr)
library(ggplot2)

args = commandArgs(trailingOnly=TRUE)


CRYSTAL_TYPE_DATA = 0x02

CRYSTAL_BAD_DATA = 1
CRYSTAL_BAD_CRC = 2
CRYSTAL_HIGH_NOISE = 3
CRYSTAL_SILENCE = 4
CRYSTAL_TX = 5


ssum = read.table("send_summary.log", header=T, colClasses=c("numeric"))
rsum = read.table("recv_summary.log", header=T, colClasses=c("numeric"))
ta_log = read.table("ta.log", header=T, colClasses=c("numeric"))
asend = read.table("app_send.log", header=T, colClasses=c("numeric"))
energy = read.table("energy.log", header=T, colClasses=c("numeric"))
energy_tf = read.table("energy_tf.log", header=T, colClasses=c("numeric"))
energy = merge(energy, energy_tf, by=c("epoch", "node"))

params = read.table("params_tbl.txt", header=T)
SINK = params$sink[1]
conc_senders = params$senders
message("Sink node: ", SINK, " concurrent senders: ", conc_senders)

payload_length = 0
if ("payload" %in% names(params)) {
    payload_length = params$payload
}
message("Payload length: ", payload_length)




min_epoch = max(c(params$start_epoch, min(rsum$epoch))+1)
max_epoch = max(rsum$epoch)-1
#max_epoch = min(params$start_epoch+params$active_epochs, max(rsum$epoch)-1)

if(length(args) == 0) {
    show_epochs = min_epoch:max_epoch
} else {
    show_epochs = as.numeric(args)
}

n_epochs = max_epoch - min_epoch + 1
message(n_epochs, " epochs: [", min_epoch, ", ", max_epoch, "]")


e = ta_log$epoch
ta_log = ta_log[e>=min_epoch & e<=max_epoch,]
e = asend$epoch
asend = asend[e>=min_epoch & e<=max_epoch,]
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

send = ta_log[ta_log$status==CRYSTAL_TX,]
recv = ta_log[ta_log$status!=CRYSTAL_TX,]

sent_anything = unique(send$src)
message("Nodes that logged sending something: ", length(sent_anything))
print(sort(sent_anything))

not_packets = (recv$type != CRYSTAL_TYPE_DATA) | (recv$status != 0)

recv_from = unique(recv[!not_packets, c("src")])
message("Messages received from the following ", length(recv_from), " nodes")
print(sort(recv_from))

message("Alive nodes that didn't work")
lazy_nodes = setdiff(alive, c(heard_from, recv_from, sent_anything, SINK))
lazy_nodes

message("Packets of wrong types or length received:")
wrong_recv = recv[not_packets & recv$bad_crc %in% c(CRYSTAL_BAD_DATA, CRYSTAL_BAD_CRC),]
wrong_recv

recv=recv[!not_packets,]

sink_recv = recv[recv$dst==SINK,]
duplicates = duplicated(sink_recv[,c("src", "seqn")])
message("Duplicates: ", sum(duplicates))

sink_recv_nodup = sink_recv[!duplicates,]
sink_rsum = rsum[rsum$dst==SINK,]

sendrecv = merge(asend[,c("src", "seqn", "epoch")], sink_recv_nodup[,c("src", "seqn", "epoch", "n_ta")], by=c("src", "seqn"), all.x=T, suffixes=c(".tx", ".rx"))
total_sent = dim(unique(send[c("src", "seqn")]))[1]
total_packets = dim(asend)[1]
nodups_recv = dim(sendrecv[!is.na(sendrecv$epoch.rx),])[1] # in sendrecv we don't have packets "received but not sent"
PDR = nodups_recv/total_sent
real_PDR = nodups_recv/total_packets
message("Total messages: ", total_packets, " sent: ", total_sent, " received: ", nodups_recv , " OLD_PDR: ", PDR, " real PDR: ", real_PDR)

lost = sendrecv[is.na(sendrecv$epoch.rx),c("src", "seqn", "epoch.tx")]
message("Not delivered packets: ", dim(lost)[1])
lost

notsent = merge(asend[,c("src", "seqn", "epoch")], send[,c("src", "seqn", "epoch")], by=c("src", "seqn"), all.x=T, suffixes=c(".app", ".cr"))
notsent = notsent[is.na(notsent$epoch.cr),]
message("Not sent packets: ", dim(notsent)[1])
print(notsent)

message("Epochs with loss:")
sort(unique(lost$epoch.tx))

message("Packets received not in the epoch they were sent")
print(sendrecv[!is.na(sendrecv$epoch.rx) & sendrecv$epoch.tx != sendrecv$epoch.rx,])

message("TX and RX records")
s = setNames(aggregate(epoch ~ ssum$src, ssum, FUN=length), c("node", "count_tx"))
r = setNames(aggregate(epoch ~ rsum$dst, rsum, FUN=length), c("node", "count_rx"))
print(merge(s,r,all=T)); rm(s); rm(r);

message("Distribution of number of consecutive misses of the sync packets from the sink")
print(setNames(aggregate(sync_missed ~ ssum$sync_missed, ssum, length), c("num_misses", "num_records")))

message("Nodes that lost a sync message more than twice in a row")
sort(unique(ssum[ssum$sync_missed>2,c("src")]))

# NA means dynamic
if (is.na(conc_senders) | conc_senders > 0) {

    message("Total and unique messages received by sink")

    rx_uniq = setNames(aggregate(
          paste(src,seqn) ~ epoch,
          sink_recv, 
            FUN=function(x) length(unique(x))),
          c("epoch", "uniq_rx"))                
    rx_dups = setNames(aggregate(
          sink_recv$src, 
          list(epoch=sink_recv$epoch), 
            FUN=function(x) (length(x) - length(unique(x)))),
          c("epoch", "dups"))
    acked = setNames(aggregate(
          send$acked, 
          list(epoch=send$epoch), 
            FUN=sum),
          c("epoch", "acks"))

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

    senders = setNames(aggregate(
          paste(src,seqn) ~ epoch,
          send,
            FUN=function(x) length(unique(x))),
          c("epoch", "n_pkt"))
    
    avg_rxta = setNames(aggregate(
          sink_recv_nodup$n_ta, 
          list(epoch=sink_recv_nodup$epoch), 
            FUN=mean),
          c("epoch", "avg_rxta"))

    avg_rxta$avg_rxta <- avg_rxta$avg_rxta + 1

    txrx = merge(senders, rx_uniq, all=T)
    txrx[is.na(txrx$uniq_rx), c("uniq_rx")] <- c(0) # no receives means zero receives

    deliv = merge(
                merge(
                    merge(
                        merge(
                            merge(
                                merge(txrx, rx_dups, all=T),
                                minta, all=T),
                            maxta, all=T),
                        acked, all=T),
                    setNames(sink_rsum[,c("epoch", "n_ta")], c("epoch", "n_ta_sink")), all=T),
                avg_rxta, all=T)

    rownames(deliv) <- deliv$epoch

    deliv[is.na(deliv$n_pkt), "n_pkt"] <- 0
    deliv[is.na(deliv$uniq_rx), "uniq_rx" ] <- 0
    deliv[is.na(deliv$dups),"dups"] <- 0
    print(deliv[,c("epoch", "n_pkt", "uniq_rx", "acks", "dups", "n_ta_sink", "n_ta_min", "n_ta_max", "avg_rxta")])

    incompl = deliv[deliv$uniq_rx<deliv$n_pkt, ]
    incompl_epochs = sort(unique(incompl$epoch))
    message("Epochs with incomplete receive: ", length(incompl_epochs))
    print(incompl_epochs)

    #message("Distribution of tx attempts required")
    #print(setNames(aggregate(epoch ~ real_send$n_tx, real_send, FUN=length), c("n_tx", "cases")))
}


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
en_short_a = within(en_short_a, ratio_short_a <- n_short_a/n_ta)

tf_s_pernode = setNames(aggregate(tf_s ~ en_short_s$node, en_short_s, mean), c("node", "tf_s"))
tf_t_pernode = setNames(tryCatch(aggregate(tf_t ~ en_short_t$node, en_short_t, mean), error=function(e) data.frame(matrix(vector(), 0, 2))), c("node", "tf_t"))
tf_a_pernode = setNames(tryCatch(aggregate(tf_a ~ en_short_a$node, en_short_a, mean), error=function(e) data.frame(matrix(vector(), 0, 2))), c("node", "tf_a"))

energy_pernode = merge(energy_pernode, tf_s_pernode, all.x=T)
energy_pernode = merge(energy_pernode, tf_a_pernode, all.x=T)
energy_pernode = merge(energy_pernode, tf_t_pernode, all.x=T)

message("Radio ON per node")
energy_pernode

write.table(energy_pernode, "pernode_energy.txt", row.names=F)

if (is.nan(PDR)) PDR = NA



indriya_coords = read.table(header=TRUE, text = "
node x        y
  1 385.23710 773.4375
  2 410.42460 756.1250
  3 417.04960 737.1250
  4 363.23710 774.3125
  5 363.36210 717.6875
  6 338.73710 722.0625
  7 446.48710 756.0625
  8 378.54960 794.0625
  9 446.36210 737.0625
 10 315.52900 689.1550
 11 416.30970 815.1875
 12 414.72420 791.1875
 13 377.41170 815.0625
 14 315.40400 739.0925
 15 498.59920 737.1875
 16 436.28670 774.1875
 17 302.15400 703.0925
 18 302.40400 750.0925
 19 448.78670 794.3125
 20 312.28670 777.8125
 21 459.78670 815.3125
 22 477.97420 774.3125
 23 132.02900 726.9675
 24 205.02900 747.3425
 25 125.59920 772.9375
 26 204.90400 708.0925
 27 174.96650 672.2175
 28 144.09150 750.3425
 29  84.02898 740.0925
 30  91.65398 689.1550
 31  84.15398 773.1550
 32 226.15400 736.9050
 33 184.84920 773.0625
 34 220.28670 773.1875
 35 225.90400 689.3425
 36 235.02900 708.3425
 37 234.95200 669.0443
 38 306.37270 427.8710
 39 285.46280 437.1716
 40 314.48520 443.5365
 41 309.09160 457.6126
 42 333.85650 443.4673
 43 294.20540 445.6331
 44 295.32130 409.0657
 45 348.61210 442.2791
 46 330.96910 472.2161
 47 348.68130 428.6425
 48 273.92260 459.9771
 49 285.46120 422.4226
 50 283.86140 466.5835
 51 273.92110 409.0657
 52 368.03340 443.4042
 53 295.39050 395.4291
 54 283.89920 492.9858
 55 335.36840 492.9858
 56 348.66200 457.5821
 57 299.14690 510.8255
 58 259.37450 492.9072
 59 335.26640 508.6418
 60 422.92320 434.1714
 61 435.28030 445.6803
 62 428.21170 463.2944
 63 439.34090 453.6452
 64 428.23090 454.9498
 65 445.46540 444.5606
 66 441.54180 466.6149
 67 393.95650 488.8142
 68 414.69650 417.2371
 69 403.95650 448.3279
 70 464.34570 437.1507
 71 489.27380 474.5154
 72 464.38410 472.2638
 73 445.52290 489.8299
 74 380.31240 462.1586
 75 388.71340 428.7707
 76 476.95020 461.0577
 77 464.40330 502.9558
 78 480.82100 428.6308
 79 433.69200 507.2110
 80 481.95020 488.7390
 81 511.50520 429.6074
 82 195.26800 169.5522
 83 230.57910 177.7195
 84 241.14060 170.8010
 85 245.1365 151.6544
 86 202.33430 178.9597
115 538.4342 188.477
116 503.39 149.0004
117 500.787 176.2404
118 503.2853 201.4398
119 529.3546 162.7217
120 552.5261 236.0244
121 569.3082 184.7033
122 506.9211 234.5463
123 506.9735 107.5746
124 464.2791 170.4785
125 605.5803 102.1135
126 480.041 124.7689
127 421.9553 149.2676
128 557.7321 102.2182
129 407.3531 232.9245
130 497.5883 118.5949
131 469.5831 146.1946
132 438.5331 99.6182
133 462.5459 199.3535
134 479.9429 234.6053
135 584.4619 102.1659
136 479.9886 99.2097
137 524.5732 90.9952
138 436.6336 233.1677
139 419.0754 201.4577
")


fbk_coords = read.table(header=TRUE, text = "
node x        y
1  1033 460
2  897 410 
3  847 373 
4  1008 361
5  1008 200
6  1033 274
7  1045 385
8  1058 361
9  1132 385
10 884 732 
11 191 447 
12 1008 732
13 1021 621
14 1045 571
15 1070 744
16 1045 695
17 1107 707
18 637 732 
19 624 856 
20 674 806 
21 736 819 
22 513 732 
23 488 707 
24 662 633 
25 575 559 
26 711 732 
27 798 707 
28 216 732 
29 92 744  
30 67 707  
31 166 707 
32 166 584 
33 191 633 
34 265 707 
35 365 732 
36 67 385  
37 129 361 
38 265 410 
39 241 299 
40 191 225 
41 934 707 
42 204 361 
43 253 373 
44 575 423 
45 513 361 
46 488 385 
47 439 361 
48 513 274 
49 612 385 
50 736 373 
")

fbk_coords = within(fbk_coords, y <- max(y) - y)

node_coords = fbk_coords
#node_coords = indriya_coords

xmin = min(node_coords$x)
xmax = max(node_coords$x)
ymin = min(node_coords$y)
ymax = max(node_coords$y)

print(node_coords)

library("ggrepel")
pdf("colors.pdf", 15, 10)

#colors=c("red", "blue", "green", "darkgreen", "orange", "black")


if (file.exists("empty_t.log")) {
    no_packet_ts = read.table("empty_t.log", h=T)
} else {
    no_packet_ts = setNames(data.frame(matrix(ncol=6, nrow=0)), c("epoch", "dst", "n_ta", "length", "status", "time"))
}

for (cur_epoch in show_epochs) {
#{cur_epoch = min(ssum$epoch)
    cur_ssum = ssum[ssum$epoch == cur_epoch, ]
    cur_send = send[send$epoch == cur_epoch, ]
    cur_recv = recv[recv$epoch == cur_epoch, ]
    cur_nopkt = no_packet_ts[no_packet_ts$epoch == cur_epoch, ]
    cur_wrong = wrong_recv[wrong_recv$epoch == cur_epoch,] # includes nopkt
    max_tries = max(cur_ssum$n_tx)
    num_tas = rsum[rsum$epoch==cur_epoch, c("dst", "n_ta")]
    
    max_tas = max(num_tas$n_ta)

    all_epoch_senders = sort(unique(cur_ssum[cur_ssum$n_tx>0, "src"]))
    message("---- Epoch: ", cur_epoch, " ---- TAs: ", max_tas, " max TX attempts: ", max_tries)
    message("All senders in current epoch:")
    print(sort(all_epoch_senders))

    #colmap = data.frame(node=all_epoch_senders, color=colors[1:length(all_epoch_senders)])
    #print(colmap)

    cur_ssum$hops = cur_ssum$hops + 1 # the original value is actually relay count (hops-1)

    nodes_hops = merge(node_coords, cur_ssum[c("src", "hops")], by.x="node", by.y="src", all.x=T)
    #for (t in 0:(max_tries-1)) {
    for (t in 0:(max_tas-1)) {
        message("TA: ", t)
        message("all senders:")
        cur_ta_ntx = cur_ssum[cur_ssum$n_tx>t,]
        cur_ta_tx = cur_send[cur_send$n_ta==t,]
        print(sort(cur_ta_ntx$src))
        cur_ta_rx = cur_recv[cur_recv$n_ta==t,]
        cur_ta_wrong = within(cur_wrong[cur_wrong$n_ta==t, c("dst", "status")], wrong<-1)[c("dst", "wrong")]
        cur_ta_nopkt = cur_nopkt[cur_nopkt$n_ta==t, c("dst", "status", "length")]
        message("heard senders:")
        print(sort(unique(cur_ta_rx$src)))
        message("all receivers:")
        print(sort(cur_ta_rx$dst))
        message("bad receivers (wrong packets):")
        print(sort(cur_ta_wrong$dst))
        message("all active nodes:")
        active_nodes = num_tas[num_tas$n_ta>t,]$dst
        print(sort(active_nodes))

        nodes = merge(nodes_hops, cur_ta_ntx[c("src", "n_tx")], by.x="node", by.y="src", all.x=T)
        nodes = merge(nodes, cur_ta_tx[c("src", "seqn", "acked")], by.x="node", by.y="src", all.x=T)
        nodes = merge(nodes, cur_ta_rx[c("src", "dst")], by.x="node", by.y="dst", all.x=T)
        nodes = merge(nodes, cur_ta_wrong[c("dst", "wrong")], by.x="node", by.y="dst", all.x=T)
        nodes = merge(nodes, cur_ta_nopkt[c("dst", "status", "length")], by.x="node", by.y="dst", all.x=T)

        nodes[!is.na(nodes$n_tx), c("n_tx")] <- 1
        nodes = rename(nodes, c("n_tx"="tx"))
        nodes = within(nodes, active<-node %in% active_nodes)
        #print(nodes)
        
        nodes_tx = nodes[!is.na(nodes$tx),]
        nodes_rx = nodes[!is.na(nodes$src),]
        
        nodes_tx$node = factor(nodes_tx$node, levels=all_epoch_senders)
        nodes_rx$src = factor(nodes_rx$src, levels=all_epoch_senders)
        cur_ssum$src = factor(cur_ssum$src, levels=all_epoch_senders)

        #cur_colors = merge(data.frame(node=union(nodes_tx$node, nodes_rx$src)), colmap)
        #print(cur_colors)
        p = ggplot(data=nodes, aes(x=x, y=y, label=node)) + 
            theme_bw() +
            geom_point(data=data.frame(src=factor(all_epoch_senders), node=c(0), x=c(0), y=c(0)), aes(color=src), size=0) + # a work-around to preserve all the sender colors when not all of them are present
            geom_point(data=nodes[nodes$active, ], shape=1, size=9) +                         # empty circle for each active node
            geom_point(data=nodes_tx, aes(color=node), shape=2, size=13) +                     # triangle around each sender
            geom_point(data=nodes_tx, aes(color=node), shape=16, size=9.5) +                 # filled circle for each sender
            geom_point(data=nodes[nodes$wrong==1,], color="gray40", shape=16, size=9.5) +     # gray filled circle for bad packets
            geom_point(data=nodes_rx, aes(color=src), shape=16, size=9.5) +                 # filled circle for each receiver
            geom_point(data=nodes[nodes$node==SINK,], shape=0, size=13) +                         # square around the sink
            geom_text(size=3.5) + 
            geom_text_repel(data=nodes_tx[nodes_tx$acked==1,], size=3.5, label="A") +
            #geom_text_repel(size=2.5, aes(label=hops), box.padding = unit(0.5, "lines"), point.padding = unit(0.05, "lines"), segment.color = 'grey50', segment.size=0) +
            geom_text(size=2.5, aes(label=hops), nudge_x=15, nudge_y=15, color="gray30") +
            geom_text(data=nodes[nodes$status == CRYSTAL_HIGH_NOISE,], size=4, aes(label=length), nudge_x=0, nudge_y=-28, color="red") +
            geom_text(data=nodes[nodes$status == CRYSTAL_SILENCE,], size=4, aes(label=length), nudge_x=0, nudge_y=-28, color="blue") +
            ylim(c(ymin, ymax)) + xlim(c(xmin, xmax)) +
            #scale_color_manual(values = cur_colors) +
            ggtitle(paste("Epoch:", cur_epoch, "TA:", t)) +
            coord_fixed() +
            theme(axis.text=element_blank()) +
            guides(col = guide_legend(nrow = 1, byrow = TRUE)) +
            theme(legend.position="bottom")
        print(p)
    }
}
dev.off()

library(reshape2)

pdf("pernode_epochs.pdf", 12, 8)
for (cur_epoch in show_epochs) {
    cur_rsum = rsum[rsum$epoch==cur_epoch,]
    cur_rsum$node <- as.factor(cur_rsum$dst)
    cur_ssum = ssum[ssum$epoch==cur_epoch,]
    cur_ssum$node <- as.factor(cur_ssum$src)
    cur_ssum$hops = cur_ssum$hops + 1 # the original value is actually relay count (hops-1)
    
    # n_ta, n_rx+n_tx
    data = merge(cur_rsum[c("node", "n_ta", "n_rx")], cur_ssum[c("node", "n_tx")], all=T)
    data = within(data, n_rx_tx <- n_rx+n_tx)
    long = melt(data[c("node", "n_ta", "n_rx_tx")])
    p = ggplot(data=long, aes(x=node, y=value, fill=variable)) + 
        theme_bw() + ggtitle(paste("Epoch:", cur_epoch, "- number of TAs and TX+RX T slots")) +
        geom_bar(position = "dodge", stat="identity")
    print(p)
    # n_rx, n_tx, have_pkt
    long = melt(data[c("node", "n_rx", "n_tx")])
    p = ggplot(data=long) + 
        theme_bw() + ggtitle(paste("Epoch:", cur_epoch, "- number of TX and RX T slots, P=has packet to send")) +
        geom_bar(aes(x=node, y=value, fill=variable), position = "dodge", stat="identity") #+
        #geom_text(data=cur_ssum[cur_ssum$have_pkt>0, c("node", "have_pkt")], aes(x=node, y=1, label="P"))
    print(p)
    
    # n_acks, sync_acks
    long = melt(cur_ssum[c("node", "n_acks", "sync_acks")])
    p = ggplot(data=long, aes(x=node, y=value, fill=variable)) + 
        theme_bw() + ggtitle(paste("Epoch:", cur_epoch, "- total number of ACKs and synchronising ACKs")) +
        geom_bar(position = "dodge", stat="identity")
    print(p)
    # hops
    long = melt(cur_ssum[c("node", "hops")])
    p = ggplot(data=long, aes(x=node, y=value, fill=variable)) + 
        theme_bw() + ggtitle(paste("Epoch:", cur_epoch, "- hops")) +
        geom_bar(position = "dodge", stat="identity")
    print(p)
    # sync_missed
    long = melt(cur_ssum[c("node", "sync_missed")])
    p = ggplot(data=long, aes(x=node, y=value, fill=variable)) + 
        theme_bw() + ggtitle(paste("Epoch:", cur_epoch, "- accumulated number of consecutive missed syncronising S packets")) +
        geom_bar(position = "dodge", stat="identity")
    print(p)
    
    # -- radio on times --
    cur_en = merge(
                   en[en$epoch==cur_epoch, c("node", "ton_s", "n_short_s", "n_short_t", "n_short_a")],
                   en_tas[en_tas$epoch==cur_epoch, c("node", "ton_t", "ton_a")], all=T
                   )
    cur_en = merge(cur_en,
                   merge(en_short_s[en_short_s$epoch==cur_epoch, c("node", "tf_s")],
                         merge(en_short_t[en_short_t$epoch==cur_epoch, c("node", "tf_t")],
                               en_short_a[en_short_a$epoch==cur_epoch, c("node", "tf_a")], all=T
                               ), all=T
                         ), all=T
                   )
    cur_en$node <- as.factor(cur_en$node)

    # s_rx_cnt, s_tx_cnt, n_short_s
    long = melt(cur_rsum[c("node", "s_rx_cnt", "s_tx_cnt")])
    p = ggplot(data=long, aes(x=node, y=value, fill=variable)) + 
        theme_bw() + ggtitle(paste("Epoch:", cur_epoch, "- number of transmitted and received packets in S slot; C=complete (short) S slot")) +
        geom_bar(position = "dodge", stat="identity") +
        geom_text(data=cur_en[cur_en$n_short_s>0, c("node", "n_short_s")], aes(x=node, y=1, label="C", fill=NA))
    print(p)

    # ton_s, tf_s,
    long = melt(cur_en[c("node", "ton_s", "tf_s")])
    p = ggplot(data=long, aes(x=node, y=value, fill=variable)) + 
        theme_bw() + ggtitle(paste("Epoch:", cur_epoch, "- duration of the S slot")) +
        geom_bar(position = "dodge", stat="identity")
    print(p)
    # n_short_t
    long = melt(cur_en[c("node", "n_short_t")])
    p = ggplot(data=long, aes(x=node, y=value, fill=variable)) + 
        theme_bw() + ggtitle(paste("Epoch:", cur_epoch, "- number of short T slots")) +
        geom_bar(position = "dodge", stat="identity")
    print(p)
    # ton_t, tf_t,
    long = melt(cur_en[c("node", "ton_t", "tf_t")])
    p = ggplot(data=long, aes(x=node, y=value, fill=variable)) + 
        theme_bw() + ggtitle(paste("Epoch:", cur_epoch, "- avg. duration of all T slots and complete (short) T slots")) +
        geom_bar(position = "dodge", stat="identity")
    print(p)
    # n_short_a
    long = melt(cur_en[c("node", "n_short_a")])
    p = ggplot(data=long, aes(x=node, y=value, fill=variable)) + 
        theme_bw() + ggtitle(paste("Epoch:", cur_epoch, "- number of short A slots")) +
        geom_bar(position = "dodge", stat="identity")
    print(p)
    # ton_a, tf_a,
    long = melt(cur_en[c("node", "ton_a", "tf_a")])
    p = ggplot(data=long, aes(x=node, y=value, fill=variable)) + 
        theme_bw() + ggtitle(paste("Epoch:", cur_epoch, "- avg. duration of all A slots and complete (short) A slots")) +
        geom_bar(position = "dodge", stat="identity")
    print(p)
    # noise 
    long = melt(cur_rsum[c("node", "noise")])
    p = ggplot(data=long, aes(x=node, y=value, fill=variable)) + 
        theme_bw() + ggtitle(paste("Epoch:", cur_epoch, "- noise")) +
        geom_point()
    print(p)
    # cca_busy
    long = melt(cur_rsum[c("node", "cca_busy")])
    p = ggplot(data=long, aes(x=node, y=value, fill=variable)) + 
        theme_bw() + ggtitle(paste("Epoch:", cur_epoch, "- CCA busy counter")) +
        geom_point()
    print(p)
}
dev.off()
