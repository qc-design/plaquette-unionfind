import matplotlib.pyplot as plt
import seaborn as sns
import sys

open_file = sys.argv[1]
save_file = sys.argv[2]
title = sys.argv[3]
skip = int(sys.argv[4])

print("Reading file: " + open_file)
print("Saving file: " + save_file)
print("Title: " + title)
print("Skip: " + str(skip))

# Read data from file
with open(open_file, 'r') as file:
    data = file.readlines()
    
# Process the data
lines = {}
for line in data:
    line = line.strip().split(',')
    name = line[0]
    x = int(line[1])
    y = float(line[3]) * 1000  # Scale y-axis to milliseconds
    if name not in lines:
        lines[name] = {'x': [], 'y': []}
    lines[name]['x'].append(x)
    lines[name]['y'].append(y)

# Set seaborn style and color palette
# sns.set(style='darkgrid')
# colors = sns.color_palette('husl', len(lines))

# Plot the data
plt.figure(figsize=(8, 6))
for i, (name, line_data) in enumerate(lines.items()):
    x = line_data['x']
    y = line_data['y']
    if len(x) > 5:
        # Only plot every 5th data point
        x = x[::skip]
        y = y[::skip]
    plt.plot(x, y, marker='o', linestyle='-', linewidth=1, label=name)


# Set labels and title
plt.xlabel('Lattice Size', fontsize=12, weight='bold')  # Update x-axis label
plt.ylabel('Time per Shot (ms)', fontsize=12, weight='bold')  # Update y-axis label
plt.title(title, fontsize=14, weight='bold')

# Set legend
legend = plt.legend(loc='upper left', prop={'size': 20})
legend.get_frame().set_edgecolor('black')  # Set legend frame color
for text in legend.get_texts():
    text.set_fontsize(12)  # Set legend text size
    text.set_weight('bold')  # Set legend text weight

# Set tick labels font weight
plt.xticks(fontweight='bold')
plt.yticks(fontweight='bold')

# Set grid
plt.grid(True, linestyle='-', linewidth=0.5, alpha=0.5)  # Update linestyle and alpha


plt.tight_layout()

# Save the plot as benchmark.png with the highest quality settings
plt.savefig(save_file, dpi=300, bbox_inches='tight')
plt.show()
