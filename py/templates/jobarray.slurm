#!/bin/bash

#SBATCH --job-name={{ name }}
#SBATCH --partition=carl.p
#SBATCH --time={{ hours }}:0:0
#SBATCH --mem={{ mb }}M
#SBATCH --array=1-{{ count }}:1
#SBATCH --mail-type=END,FAIL

module load GCC
mkdir -p rawData

$(sed -n ${SLURM_ARRAY_TASK_ID}'p' {{ name }}.lst)
