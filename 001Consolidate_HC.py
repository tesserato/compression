from os import listdir
import csv
# from os.path import isfile, join
# import signal_envelope as se 
import numpy as np

def consolidate(path):
  csv_file = open(path + "1.csv")
  csv_reader = csv.reader(csv_file)
  rows = [r for r in csv_reader]
  formats = rows[0][1:]
  samples = [r[0] for r in rows[1:]]

  # print(samples)
  cube = []

  for file in listdir(path):
    
    if file.endswith(".csv"):
      csv_file = open(path + file)
      csv_reader = csv.reader(csv_file)
      rows = [r for r in csv_reader]
      matrix = []
      for row in rows[1:]:
        matrix.append(row[1:])
      matrix = np.array(matrix)
      cube.append(matrix)
      if matrix.shape != (16, 4):
        print(file)
      # print(matrix.shape)

  cube = np.array(cube, dtype=int)
  print(cube.shape)
  

  result = "sample"
  for fmt in formats:
    result += f",{fmt} AVG,{fmt} SD"
  result +="\n"

  mat = []
  for i, sample in enumerate(samples):
    result += sample
    line = []
    for j, fmt in enumerate(formats):
      avg = np.average(cube[:, i, j])
      std = np.std(cube[:, i, j])
      result+=f",{avg},{std}"
      line.append(avg)
      line.append(std)
    mat.append(line)
    result +="\n"

  mat = np.array(mat)
  result += "Average"
  for avg in np.average(mat, 0):
    result+=f",{avg}"
  result += "\nSD"
  for std in np.std(mat, 0):
    result+=f",{std}"


  return result

result = consolidate("004_results/Compression_time_milliseconds/")
f = open("004_results/Compression_time_milliseconds.csv", "w")
f.write(result)
f.close()

result = consolidate("004_results/Decompression_time_milliseconds/")
f = open("004_results/Decompression_time_milliseconds.csv", "w")
f.write(result)
f.close()