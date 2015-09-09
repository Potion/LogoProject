###Logo project

Uses raw openGL within Cinder project to allow interaction with logo.

####Installation
**Adding OpenCV**
* Include opencv static libraries (```libopencv_calib3d.a```, ```libopencv_core.a```, ```libopencv_features2d.a```, etc.) in the folder ```libs/opencv/``` at the same level as the ```Cinder/``` folder.

* Include 3rdParty static libraries (```libIlmImf.a```, ```liblibjasper.a```, etc.) in the folder ```libs/3rdParty/``` next to the opencv/ folder described above.

* Under Build Phases, add the openCV static libraries (the ones listed above) under ```Link Binary With Libraries```. (Click the plus sign and navigate to the folder added above).

* Under Build Phases, add ```QTKit.framework```. (Click the plus sign: this should show up in the default list).

* Include opencv files (```calib3d.hpp```, ```core.hpp```, other folders, etc.) in the folder ```include/opencv2/``` at the same level as the ```Cinder/``` folder.

* Add the following to the Other Linker Flags under Build Settings in XCode:
	* ```-lopencv_calib3d -lopencv_core -lopencv_features2d -lopencv_flann -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lopencv_ml -lopencv_objdetect -lopencv_photo -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_ts -lopencv_video -lopencv_videoio -lopencv_videostab```

* Add the following to the Header Search Paths under Build Settings:
	* ```../../../include```

* Add the following to the Library Search Paths under Build Settings:
	* ```../../../libs/opencv```
	* ```../../../libs/3rdParty```

