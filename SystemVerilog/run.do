# Copyright 1991-2014 Mentor Graphics Corporation
#
# All Rights Reserved.
#
# THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY INFORMATION
# WHICH IS THE PROPERTY OF MENTOR GRAPHICS CORPORATION
# OR ITS LICENSORS AND IS SUBJECT TO LICENSE TERMS.

# Use this run.do file to run this example.
# Either bring up ModelSim and type the following at the "ModelSim>" prompt:
#     do run.do
# or, to run from a shell, type the following at the shell prompt:
#     vsim -do run.do -c
# (omit the "-c" to see the GUI while running from the shell)

onbreak {resume}

if [file exists work] {
  vdel -lib ./work -all
}

echo "###"
echo "### Creating library and compiling design ..."
echo "###"

# Création de la librairie de travail
vlib work

# compile the Verilog source files
vlog *.sv

# compile and link C source files
sccom -g RSEncoder.cpp
sccom -link

# open debugging windows
quietly view *

# Lancement du simulateur et du banc de test
# La commande -t ps spécifie la résolution temporel en picoseconde au simulateur
vsim -t ps test_RSEncoder

# Affichage et configuration de la fenêtre de visualisation des traces
add wave -r *

# Execution du banc de test pour * ns
run 500000 ns
