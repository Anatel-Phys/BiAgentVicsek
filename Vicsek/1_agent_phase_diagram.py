import matplotlib
from matplotlib import pyplot as plt
import pandas
import numpy as np

filename = "polarization_2_agent.txt"

# color map for segregity : magma, color map for polariszation : viridis
data = pandas.read_csv(filename, sep='\t', header=None)
data_np = data.to_numpy()

fig, ax = plt.subplots(figsize=(8, 8))
plt.imshow(data_np, cmap='viridis')
colorbar = plt.colorbar()   
plt.show()

