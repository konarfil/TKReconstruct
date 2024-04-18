
rm -rf ./build ./src/dicts/ ./lib
mkdir build lib

echo "#############################################"
echo "#         GENERATE ROOT DICTIONARIES        #"
echo "#############################################"
echo " "

cd include
echo "rootcling: ./include/TKEvent.h         -> ./include/TKEventdict_rdict.pcm         + ./include/TKEventdict.cpp"
      	      rootcling -f ../lib/TKEventdict.cpp  TKEvent.h+
echo "rootcling: ./include/TKOMhit.h         -> ./include/TKOMhitdict_rdict.pcm         + ./include/TKOMhitdict.cpp"
      	      rootcling -f ../lib/TKOMhitdict.cpp  TKOMhit.h+
echo "rootcling: ./include/TKtrhit.h         -> ./include/TKtrhitdict_rdict.pcm         + ./include/TKtrhitdict.cpp"
      	      rootcling -f ../lib/TKtrhitdict.cpp  TKtrhit.h+
echo "rootcling: ./include/TKtrack.h         -> ./include/TKtrackdict_rdict.pcm         + ./include/TKtrackdict.cpp"
      	      rootcling -f ../lib/TKtrackdict.cpp  TKtrack.h+
echo "rootcling: ./include/TKcluster.h       -> ./include/TKclusterdict_rdict.pcm       + ./include/TKclusterdict.cpp"
      	       rootcling -f ../lib/TKclusterdict.cpp  TKcluster.h+
      	echo "rootcling: ./include/TKtrajectory.h     -> ./include/TKtrajectorydict_rdict.pcm       + ./include/TKtrajectorydict.cpp"
      	      rootcling -f ../lib/TKtrajectorydict.cpp  TKtrajectory.h+
      	echo "rootcling: ./include/TKpoint.h     -> ./include/TKpointdict_rdict.pcm       + ./include/TKpointdict.cpp"
      	      rootcling -f ../lib/TKpointdict.cpp  TKpoint.h+
echo " "
echo "Dictionaries generated!"
echo " "
	cd ..

	cp -R ./lib/ ./src/dicts/
	rm -rf ./lib/*.cpp
	rm -rf ./src/dicts/*.pcm

echo "#############################################"
echo "#         STARTING CMAKE                    #"
echo "#############################################"
echo " "

cd build

	# cmake -DCMAKE_PREFIX_PATH=$FAL_INC ..
	cmake -DCMAKE_PREFIX_PATH=/sps/nemo/sw/Falaise/tests/install_244/include/ ..
	make

cd ..

