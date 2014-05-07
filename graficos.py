import matplotlib
matplotlib.use('Agg')
import sys

import pylab as pl

file_name = (sys.argv[1].split("/")[1]).split(".txt")[0]
dados = open(sys.argv[1],"r")
dir_dados = sys.argv[2]

lines = dados.readlines()
lines.pop(0)
x = []
y = []

for l in lines:
    d = l.split()
    x.append(100*float(d[0]))
    y.append(100*float(d[1]))

pl.plot(x, y, marker='o',linestyle='-',color='b')
pl.title('Precision-Recall curve: '+file_name)
pl.xlabel('Recall')
pl.ylabel('Precision')
pl.ylim([-10,120])
pl.xlim([-0.5,100])
#pl.show()

pl.savefig(dir_dados + file_name + '.png')

dados.close()
