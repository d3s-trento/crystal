powers = [31]
channels = [26]
sinks = [1]
testbed = "fbk"
start_epoch = 1
active_epochs = 200
full_epochs = 15
#num_senderss = [0,1,2,5,10,20]
num_senderss = [1,20]
period = 1

n_tx_s = 3
dur_s  = 10

n_tx_t = 3
dur_t  = 8

n_tx_a = 3
dur_a  = 8

payload = 2

longskips = [0]
n_emptys = [(2,2,4,6)]
ccas = [(-15, 80)]

nodemaps=["all"]
#nodemaps=["all", "n7_MWO", "n13_MWO", "n42_MWO", "n7_12_20_31_37_49_WIFI4"]

chmap = "nomap"
boot_chop = "hop3"
#chmap = "nohop"
#boot_chop = "nohop"

logging = True
seed = 123
