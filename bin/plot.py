import matplotlib.pyplot as plt
import numpy as np
import random
import argparse

argparser = argparse.ArgumentParser()
argparser.add_argument( '-v','--vertices', type=str, help='Path to vertices file', default='vertices.txt')
argparser.add_argument('-e','--edges', type=str, help='Path to edges file', default='edges.txt')
argparser.add_argument('-o','--output', type=str, help='Path to output file', default='output.png')
argparser.add_argument('-t','--title', type=str, help='Title of the plot', default='Syndrome Graph')
argparser.add_argument('-inv','--invert_y_axis', type=bool, help='Invert Y axis', default=True)
argparser.add_argument('-show','--show_plot', type=bool, help='Show Plot', default=False)

args = vars(argparser.parse_args())
edges_file = args['edges']
vertices_file = args['vertices']
output = args['output']
show = args['show_plot']

fig, ax = plt.subplots()
labels = set()

edges = []
vertices = []

class Edge:
    def __init__(self, x0=0, x1=0, y0=0, y1=0, label=None, linestyle='solid', c='b', linewidth=1, alpha=0.5, fraction=1.0,annotation=""):
        self.x0 = x0
        self.x1 = x1
        self.x0 = y0
        self.x1 = y1
        self.linewidth = linewidth
        self.linestyle = linestyle
        self.alpha = alpha
        self.label = label
        self.c = c
        self.fraction = fraction
        self.annotation = annotation
        
class Vertex:
    def __init__(self, x=0, y=0, label=None, marker='o', c='r', markersize=10, fillstyle='full',annotation=""):
        self.x = x
        self.y = y
        self.markersize = markersize
        self.fillstyle = fillstyle
        self.label = label
        self.c = c
        self.marker = marker
        self.annotation = annotation
        
edge_labels = set()
vertex_labels = set()

with open(edges_file) as f:
    for line in f:
        fields = [x for x in line.split()]
        edge = Edge()
        edge.x0 = float(fields[0])
        edge.x1 = float(fields[1])
        edge.y0 = float(fields[2])
        edge.y1 = float(fields[3])
        if len(fields) > 4:
            if fields[4] not in edge_labels:
                edge.label = fields[4]
                edge_labels.add(fields[4])
                string_with_underscore = edge.label
                string_with_space = string_with_underscore.replace("_", " ")
                edge.label = string_with_space
            else:
                edge.label = None
        if len(fields) > 5:
            edge.linestyle = fields[5]
        if len(fields) > 6:
            edge.c = fields[6]
        if len(fields) > 7:
            edge.linewidth = float(fields[7])
        if len(fields) > 8:
            edge.alpha = float(fields[8])
        if len(fields) > 9:
            edge.fraction = float(fields[9])
        if len(fields) > 10:
            edge.annotation = float(fields[10])
        edges.append(edge)

with open(vertices_file) as f:
    for line in f:
        fields = [x for x in line.split()]
        vertex = Vertex()
        vertex.x = float(fields[0])
        vertex.y = float(fields[1])
        if len(fields) > 2:
            if fields[2] not in vertex_labels:
                vertex.label = fields[2]               
                vertex_labels.add(fields[2])
                string_with_underscore = vertex.label
                string_with_space = string_with_underscore.replace("_", " ")
                vertex.label = string_with_space
            else:
                vertex.label = None
        if len(fields) > 3:
            vertex.marker = fields[3]
        if len(fields) > 4:
            vertex.c = fields[4]
        if len(fields) > 5:
            vertex.markersize = float(fields[5])
        if len(fields) > 6:
            vertex.fillstyle = fields[6]
        if len(fields) > 7:
            vertex.annotation = fields[7]
        vertices.append(vertex)


for i in range(len(edges)):
    edge = edges[i]

    # Calculate the length of the line
    length = np.sqrt((edge.x1 - edge.x0)**2 + (edge.y1 - edge.y0)**2)

    # Calculate the endpoint of the visible part of the line
    x_end = edge.x0 + edge.fraction * (edge.x1 - edge.x0)
    y_end = edge.y0 + edge.fraction * (edge.y1 - edge.y0)

    edge_half_x = (edge.x0 + x_end) * .5
    edge_half_y = (edge.y0 + y_end) * .5

    
    ax.plot([edge.x0, x_end],
            [edge.y0, y_end],
            linestyle=edge.linestyle,
            c=edge.c,
            label=edge.label,
            linewidth=edge.linewidth,
            alpha=edge.alpha)
    if (edge.annotation != ""):
        ax.annotate(str(edge.annotation).replace(".0",""), xy=(edge_half_x, edge_half_y), xytext=(edge_half_x, edge_half_y))

    
x = set()
y = set()

for i in range(len(vertices)):
    vertex = vertices[i]
    x.add(vertex.x)
    y.add(vertex.y)
    ax.plot(vertex.x, vertex.y, vertex.marker, c=vertex.c, markersize=vertex.markersize, label=vertex.label, fillstyle=vertex.fillstyle)
    if (vertex.annotation != ""):
        ax.annotate(vertex.annotation, xy=(vertex.x, vertex.y), xytext=(vertex.x, vertex.y))


x_sorted = np.array(list(x))
y_sorted = np.array(list(y))
x_sorted.sort()
y_sorted.sort()

#ax.scatter(xv, yv, c='r', s=50)
if args['invert_y_axis']:
    plt.gca().invert_yaxis()
    ax.xaxis.set_ticks_position('top')

# Add x and y axis labels as integer locations of the vertices
ax.set_xticks(x_sorted)
ax.set_xticklabels(x_sorted)
ax.set_yticks(y_sorted)
ax.set_yticklabels(y_sorted)

plt.legend(loc='center left', bbox_to_anchor=(1.05, .5), prop={'size': 6}, labelspacing=1.5, borderpad=1.5)
#ax.set_title(args['title'], pad=20)

fig.suptitle(args['title'], y=.975, fontsize=16)
plt.tight_layout(pad=2)
plt.savefig(output)

if show:
    plt.show()
