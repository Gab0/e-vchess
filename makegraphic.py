#!/bin/python
import sys
try:
    import matplotlib.pyplot as plt
except:
    def show_mem_graphic():
        print("no matplotlib lib.")
def show_mem_graphic():
    source = open('mempertime', 'r')


    plt.xlim = (0, 2000000)
    plt.ylim = (0, 30000)

    plt.xlabel("round number")
    plt.ylabel("memory usage KB")

    plt.suptitle("Round number VS Memory usage")

    #plt.text(3000,-10,"round number")
    #plt.text(-10, 10000, "size in KB")
    plt.tight_layout()
    data = source.readlines()

    # def makedot(xy, style):

    for point in data:
        Info = point.split('|')
        coord = [int(a) for a in Info[:2]]

        DOT = 'ro'
        if len(Info) > 2:

            DOT = Info[2][:2]

        #print("point x: %i    y: %i" % (coord[0], coord[1]))
        plt.plot([coord[0]], [coord[1]], DOT)


    plt.show()
if "show" in sys.argv:
    show_mem_graphic()
