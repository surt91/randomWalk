#!/bin/bash

#SBATCH --job-name={{ name }}
#SBATCH --partition=carl.p
#SBATCH --time={{ hours }}:0:0
#SBATCH --mem={{ mb }}M
#SBATCH --ntasks={{ numTemperatures }}
#SBATCH --mail-type=END,FAIL

module load gompi
mkdir -p rawData

{{ command }}
