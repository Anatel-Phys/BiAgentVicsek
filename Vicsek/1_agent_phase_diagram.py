import matplotlib
from matplotlib import pyplot as plt
import pandas
import numpy as np

#color map for segregity : magma, color map for polariszation : viridis

data = pandas.read_csv("segregity_more_detailed.txt", sep='\t', header=None)
data_np = data.to_numpy()

fig, ax = plt.subplots(figsize=(8, 8))
plt.imshow(data_np, cmap='magma')
colorbar = plt.colorbar()   
plt.show()

