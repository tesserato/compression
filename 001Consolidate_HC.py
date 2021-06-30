from os import listdir
import csv
import numpy as np

''''
This script consolidates the results of the compressions and decompressions obtained by running 001Compress_HC.ps1
'''

def consolidate(path):
  csv_file = open(path + "1.csv")
  csv_reader = csv.reader(csv_file)
  rows = [r for r in csv_reader]
  formats = rows[0][1:]
  samples = [r[0] for r in rows[1:]]
  cube = []

  n_files = 0
  for file in listdir(path):
    if file.endswith(".csv"):
      n_files += 1
      csv_file = open(path + file)
      csv_reader = csv.reader(csv_file)
      rows = [r for r in csv_reader]
      matrix = []
      for row in rows[1:]:
        matrix.append(row[1:])
      matrix = np.array(matrix)
      cube.append(matrix)

  cube = np.array(cube, dtype=int) 

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
  return result, n_files

result, n_files = consolidate("004_results_HC/Compression_time_milliseconds/")
f = open(f"004_results_HC/Compression_time_milliseconds_{n_files}_runs.csv", "w")
f.write(result)
f.close()

result, n_files = consolidate("004_results_HC/Decompression_time_milliseconds/")
f = open(f"004_results_HC/Decompression_time_milliseconds_{n_files}_runs.csv", "w")
f.write(result)
f.close()