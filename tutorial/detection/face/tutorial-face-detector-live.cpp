//! \example tutorial-face-detector-live.cpp
#include <visp/vpDisplayGDI.h>
#include <visp/vpDisplayOpenCV.h>
#include <visp/vpDisplayX.h>
#include <visp/vpDetectorFace.h>
#include <visp/vpV4l2Grabber.h>

int main(int argc, const char* argv[])
{
#ifdef VISP_HAVE_OPENCV
  try {
    std::string opt_face_cascade_name = "./haarcascade_frontalface_alt.xml";

    int opt_device = 0;

    for (int i=0; i<argc; i++) {
      if (std::string(argv[i]) == "--haar")
        opt_face_cascade_name = std::string(argv[i+1]);
      else if (std::string(argv[i]) == "--device")
        opt_device = atoi(argv[i+1]);
      else if (std::string(argv[i]) == "--help") {
        std::cout << "Usage: " << argv[0] << " [--device <camera device>] [--haar <haarcascade xml filename>] [--help]" << std::endl;
        return 0;
      }
    }

    vpImage<unsigned char> I; // for gray images

#if defined(VISP_HAVE_V4L2)
    vpV4l2Grabber g;
    std::ostringstream device;
    device << "/dev/video" << opt_device;
    g.setDevice(device.str());
    g.setScale(2);
    g.acquire(I);
#elif defined(VISP_HAVE_OPENCV)
    cv::VideoCapture cap(opt_device); // open the default camera
    if(!cap.isOpened()) { // check if we succeeded
      std::cout << "Failed to open the camera" << std::endl;
      return -1;
    }
    cv::Mat frame;
    cap >> frame; // get a new frame from camera
    vpImageConvert::convert(frame, I);
#endif

#if defined(VISP_HAVE_X11)
    vpDisplayX d(I);
#elif defined(VISP_HAVE_GDI)
    vpDisplayDGI d(I);
#elif defined(VISP_HAVE_OPENCV)
    vpDisplayOpenCV d(I);
#endif
    vpDisplay::setTitle(I, "ViSP viewer");

    vpDetectorFace face_detector;
    face_detector.setCascadeClassifierFile(opt_face_cascade_name);

    while(1) {
      double t = vpTime::measureTimeMs();
#if defined(VISP_HAVE_V4L2)
      g.acquire(I);
#else
      cap >> frame; // get a new frame from camera
      vpImageConvert::convert(frame, I);
#endif

      vpDisplay::display(I);
      bool face_found = face_detector.detect(I);

      if (face_found) {
        std::ostringstream text;
        text << "Found " << face_detector.getNbObjects() << " face(s)";
        vpDisplay::displayText(I, 10, 10, text.str(), vpColor::red);
        for(size_t i=0; i < face_detector.getNbObjects(); i++) {
          std::vector<vpImagePoint> p = face_detector.getPolygon(i);
          vpRect bbox = face_detector.getBBox(i);
          vpDisplay::displayRectangle(I, bbox, vpColor::green, false, 4);
          vpDisplay::displayText(I, bbox.getTop()-10, bbox.getLeft(), "Message: \"" + face_detector.getMessage(i) + "\"", vpColor::red);
        }
      }
      vpDisplay::displayText(I, I.getHeight()-25, 10, "Click to quit...", vpColor::red);
      vpDisplay::flush(I);
      if (vpDisplay::getClick(I, false)) // a click to exit
        break;

      std::cout << "Loop time: " << vpTime::measureTimeMs() - t << " ms" << std::endl;
    }
  }
  catch(vpException &e) {
    std::cout << e.getMessage() << std::endl;
  }
#else
  (void)argc;
  (void)argv;
#endif
}
