import imageio
from pathlib import Path
import os

imgs = []
files = sorted(Path('display/image/').iterdir(), key=os.path.getmtime)
ims = [str(i) for i in files if str(i).endswith('.png')]
for i in ims:
	imgs.append(imageio.imread(i))
imageio.mimsave('display/image/movie.gif', imgs, fps=5)