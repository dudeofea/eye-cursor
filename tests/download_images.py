from subprocess import check_output
import os

if not os.path.exists('test_images/muct_faces/muct'):
	check_output('mkdir test_images/muct_faces/muct', shell=True)
	print check_output('git clone https://github.com/StephenMilborrow/muct test_images/muct_faces/muct', shell=True)
	print check_output('cd test_images/muct_faces/muct && for file in *.tar.gz; do tar -zxf $file; done', shell=True)

#TODO: download LFW faces (http://vis-www.cs.umass.edu/lfw/lfw.tgz)
