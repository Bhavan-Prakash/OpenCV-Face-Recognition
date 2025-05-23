// #include <iostream>
// #include <opencv2/opencv.hpp>
// #include "TMtCNN.h"
// #include "TArcface.h"
// #include "TRetina.h"
// #include "TWarp.h"
// #include "TLive.h"
// #include "TBlur.h"
// #include <queue>
// #include <set>
// #include <ctime>
// #include <map>

// //----------------------------------------------------------------------------------------
// // Creators: Arjun Charak, Aditya Raina
// //----------------------------------------------------------------------------------------
// // Build defines
// // comment them to turn a function off
// //----------------------------------------------------------------------------------------
// #define RETINA                  //comment if you want to use MtCNN landmark detection instead
// #define RECOGNIZE_FACE
// //#define TEST_LIVING
// #define AUTO_FILL_DATABASE
// //#define BLUR_FILTER_STRANGER
// // some diagnostics
// #define SHOW_LEGEND
// #define SHOW_LANDMARKS



// // Struct to hold employee information
// struct EmployeeInfo {
//     std::string emp_code;
//     std::time_t timestamp;

// };



// // Function to make curl request with emp_code
// void makeCurlRequest(const std::string& emp_code, std::map<std::string, std::time_t>& lastRequestTime, std::map<std::string, int>& curlRequestCount, std::queue<std::string>& failedRequests) {
//     std::time_t currentTime = std::time(nullptr);

//     // Check if a request has been made for this emp_code within the last 30 minutes
//     if (lastRequestTime.find(emp_code) != lastRequestTime.end()) {
//         std::time_t lastRequest = lastRequestTime[emp_code];
//         if (currentTime - lastRequest <= 30 * 60) {
//             // Request already made within the last 30 minutes
//             std::cerr << "Curl request already made for employee code " << emp_code << " within the last 30 minutes. Skipping." << std::endl;
//             return; // Skip making the curl request
//         }
//     }

//     std::string apiKey = "qwerty!";
//     std::string checkin = "Check-in";
//     std::string postData = "apikey=" + apiKey +"&atype=" + checkin + "&emp_code=" + emp_code;
//     std::string curlCommand = "curl -X POST -d \"" + postData + "\" https://trinidad-badge-subcommittee-cute.trycloudflare.com/attendance/attendance.php";

//     // Try to make the curl request
//     FILE* pipe = popen(curlCommand.c_str(), "r");
//     if (!pipe) {
//         // Error handling: Unable to execute the curl command
//         std::cerr << "Error: popen failed." << std::endl;
//         if (lastRequestTime.find(emp_code) == lastRequestTime.end()) {
//             // If no request has been made for emp_code in the last 30 minutes and the curl request fails, push emp_code into the queue of failed requests
//             failedRequests.push(emp_code);
//         }
//         return;
//     }

//     char buffer[128];
//     std::string result = "";
//     while (!feof(pipe)) {
//         if (fgets(buffer, 128, pipe) != nullptr)
//             result += buffer;
//     }
//     int curlResult = pclose(pipe);

//     if (curlResult != 0) {
//         // Error handling: curl command failed
//         std::cerr << "Error: Curl request failed for employee code " << emp_code << std::endl;
//         return;
//     } else if (result.find("Success") != std::string::npos) {
//         // Assuming successful response from the server
//         std::cerr << "Success: POST request using cURL" << std::endl;

//         // Update the last request time for this employee code
//         lastRequestTime[emp_code] = currentTime;

//         // Update the count for this employee code
//         curlRequestCount[emp_code]++;

//         // Print the count
//         std::cerr << "Curl requests made for employee code " << emp_code << ": " << curlRequestCount[emp_code] << std::endl;
//     } else {
//         // Error handling: Server-side error
//         std::cerr << "Success: POST request using cURL: " << emp_code << std::endl;
//         return;
//     }
// }





// // Function to process the queue
// void processQueue(std::queue<EmployeeInfo>& emp_queue, std::set<std::string>& processed_codes, std::map<std::string, std::time_t>& lastRequestTime, std::map<std::string, int>& curlRequestCount, std::queue<std::string>& failedRequests) {
//     const int TIME_FRAME = 60 * 30; // Time frame in seconds (30 minutes)
//     std::time_t currentTime = std::time(nullptr);

//     // Process the queue until it's empty or the time frame has elapsed
//     while (!emp_queue.empty()) {
//         if (currentTime - emp_queue.front().timestamp <= TIME_FRAME && processed_codes.find(emp_queue.front().emp_code) == processed_codes.end()) {
//             // If emp_code hasn't been processed within the time frame, make a curl request
//            // makeCurlRequest(emp_queue.front().emp_code, lastRequestTime, curlRequestCount, failedRequests);
//             processed_codes.insert(emp_queue.front().emp_code); // Add emp_code to processed set
//         }
//         emp_queue.pop(); // Remove the processed emp_code from the queue
//     }
// }


// //----------------------------------------------------------------------------------------
// // Adjustable Parameters
// //----------------------------------------------------------------------------------------
// const int   MaxItemsDatabase = 2000;
// const int   MinHeightFace    = 50;
// const float MinFaceThreshold = 0.55;
// const float FaceLiving       = 0.20;
// const double MaxBlur         = -20.0;   //more positive = sharper image
// const double MaxAngle        = 41.0;
// //----------------------------------------------------------------------------------------
// // Some globals
// //----------------------------------------------------------------------------------------
// const int   RetinaWidth      = 324;
// const int   RetinaHeight     = 240;
// float ScaleX, ScaleY;
// vector<std::string> NameFaces;
// //----------------------------------------------------------------------------------------
// using namespace std;
// using namespace cv;
// //----------------------------------------------------------------------------------------
// //  Computing the cosine distance between input feature and ground truth feature
// //----------------------------------------------------------------------------------------
// inline float CosineDistance(const cv::Mat &v1, const cv::Mat &v2)
// {
//     double dot = v1.dot(v2);
//     double denom_v1 = norm(v1);
//     double denom_v2 = norm(v2);
//     return dot / (denom_v1 * denom_v2);
// }
// //----------------------------------------------------------------------------------------
// // painting
// //----------------------------------------------------------------------------------------
// void DrawObjects(cv::Mat &frame, vector<FaceObject> &Faces)
// {
//     for(size_t i=0; i < Faces.size(); i++){
//         FaceObject& obj = Faces[i];

// //----- rectangle around the face -------
//         obj.rect.x *= ScaleX;
//         obj.rect.y *= ScaleY;
//         obj.rect.width *= ScaleX;
//         obj.rect.height*= ScaleY;
//         cv::rectangle(frame, obj.rect, cv::Scalar(0, 255, 0));
// //---------------------------------------

// //----- diagnostic ----------------------
// #ifdef SHOW_LANDMARKS
//         for(int u=0;u<5;u++){
//             obj.landmark[u].x*=ScaleX;
//             obj.landmark[u].y*=ScaleY;
//         }

//         cv::circle(frame, obj.landmark[0], 2, cv::Scalar(0, 255, 255), -1);
//         cv::circle(frame, obj.landmark[1], 2, cv::Scalar(0, 255, 255), -1);
//         cv::circle(frame, obj.landmark[2], 2, cv::Scalar(0, 255, 255), -1);
//         cv::circle(frame, obj.landmark[3], 2, cv::Scalar(0, 255, 255), -1);
//         cv::circle(frame, obj.landmark[4], 2, cv::Scalar(0, 255, 255), -1);
// #endif // SHOW_LANDMARKS
// //---------------------------------------
// #ifdef SHOW_LEGEND
//         cv::putText(frame, cv::format("Angle : %0.1f", obj.Angle),cv::Point(10,40),cv::FONT_HERSHEY_SIMPLEX,0.6, cv::Scalar(180, 180, 0));
//         cv::putText(frame, cv::format("Face prob : %0.4f", obj.FaceProb),cv::Point(10,60),cv::FONT_HERSHEY_SIMPLEX,0.6, cv::Scalar(180, 180, 0));
//         cv::putText(frame, cv::format("Name prob : %0.4f", obj.NameProb),cv::Point(10,80),cv::FONT_HERSHEY_SIMPLEX,0.6, cv::Scalar(180, 180, 0));
// #ifdef TEST_LIVING
//         if(obj.Color==2){
//             //face is too tiny
//             cv::putText(frame, cv::format("Live prob : ??"),cv::Point(10,100),cv::FONT_HERSHEY_SIMPLEX,0.6, cv::Scalar(180, 180, 0));
//         }
//         else{
//             //face is ok
//             cv::putText(frame, cv::format("Live prob : %0.4f", obj.LiveProb),cv::Point(10,100),cv::FONT_HERSHEY_SIMPLEX,0.6, cv::Scalar(180, 180, 0));
//         }
// #endif // TEST_LIVING
// #endif // SHOW_LEGEND
// //----- labels ----------------------------
// #ifdef RECOGNIZE_FACE
//         std::string Str;
//         cv::Scalar color;
//         int  baseLine = 0;

//         switch(obj.Color){
//             case 0 : color = cv::Scalar(255, 255, 255); break;  //default white -> face ok
//             case 1 : color = cv::Scalar( 80, 255, 255); break;  //yellow ->stranger
//             case 2 : color = cv::Scalar(255, 237, 178); break;  //blue -> too tiny
//             case 3 : color = cv::Scalar(127, 127, 255); break;  //red -> fake
//             default: color = cv::Scalar(255, 255, 255);
//         }

//         switch(obj.NameIndex){
//             case -1: Str="Stranger"; break;
//             case -2: Str="too tiny"; break;
//             case -3: Str="Fake !";   break;
//             default: Str=NameFaces[obj.NameIndex];
//         }

//         cv::Size label_size = cv::getTextSize(Str, cv::FONT_HERSHEY_SIMPLEX, 0.6, 1, &baseLine);
//         int x = obj.rect.x;
//         int y = obj.rect.y - label_size.height - baseLine;
//         if(y<0) y = 0;
//         if(x+label_size.width > frame.cols) x=frame.cols-label_size.width;

//         cv::rectangle(frame, cv::Rect(cv::Point(x, y), cv::Size(label_size.width, label_size.height + baseLine)),color, -1);
//         cv::putText(frame, Str, cv::Point(x, y+label_size.height+2),cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 0));
// #endif // RECOGNIZE_FACE
//     }
// }
// //----------------------------------------------------------------------------------------
// // main
// //----------------------------------------------------------------------------------------
// int main(int argc, char **argv)
// {
//     // Initialize lastRequestTime as an empty map
//     std::map<std::string, std::time_t> lastRequestTime;
//     std::map<std::string, int> curlRequestCount;
//     std::queue<std::string> failedRequests;
//     float f;
//     float FPS[16];
//     int n, Fcnt = 0;
//     size_t i;
//     cv::Mat frame;
//     cv::Mat result_cnn;
//     cv::Mat faces;
//     std::vector<FaceObject> Faces;
//     vector<cv::Mat> fc1;
//     string pattern_jpg = "./img/*.jpg";
//     std::string NewItemName;
//     size_t FaceCnt;
//     // The networks
//     TLive Live;
//     TWarp Warp;
//     TMtCNN MtCNN;
//     TArcFace ArcFace;
//     TRetina Rtn(RetinaWidth, RetinaHeight, true); // Have Vulkan support on a Jetson Nano
//     TBlur Blur;
//     // Some timing
//     chrono::steady_clock::time_point Tbegin, Tend;
//     std::queue<EmployeeInfo> emp_queue;  // Queue to hold emp_codes
//     std::set<std::string> processed_codes;  // Set to track processed emp_codes

//     Live.LoadModel();

//     for (i = 0; i < 16; i++) FPS[i] = 0.0;

//     // OpenCV Version
//     cout << "OpenCV Version: " << CV_MAJOR_VERSION << "." << CV_MINOR_VERSION << "." << CV_SUBMINOR_VERSION << endl;
//     cout << " " << endl;

//     #ifdef RECOGNIZE_FACE
//     cout << "Trying to recognize faces" << endl;
//     cout << " " << endl;
//     #ifdef RETINA
//     cout << "Using Retina" << endl;
//     cout << " " << endl;
//     #else
//     cout << "Using MtCNN" << endl;
//     cout << " " << endl;
//     #endif // RETINA

//     #ifdef TEST_LIVING
//     cout << "Test living or fake face" << endl;
//     cout << " " << endl;
//     #endif // TEST_LIVING

//     #ifdef AUTO_FILL_DATABASE
//     cout << "Automatic adding strangers to database" << endl;
//     cout << " " << endl;
//     #ifdef BLUR_FILTER_STRANGER
//     cout << "Blur filter - only sharp images to database" << endl;
//     cout << " " << endl;
//     #endif // BLUR_FILTER_STRANGER
//     #endif // AUTO_FILL_DATABASE
//     #endif // RECOGNIZE_FACE

//     // If you like to load a picture of a face into the database
//     // give the name of the .jpg image as argument on the command line
//     // without arguments the app will run the .mp4 video or use the camera
//     if (argc > 1) {
//         const char* imagepath = argv[1];

//         cv::Mat frame = cv::imread(imagepath, 1);
//         if (frame.empty()) {
//             fprintf(stderr, "cv::imread %s failed\n", imagepath);
//             return -1;
//         }
//         // Extract
//         ScaleX = ((float) frame.cols) / RetinaWidth;
//         ScaleY = ((float) frame.rows) / RetinaHeight;
//         // Copy/resize image to result_cnn as input tensor
//         cv::resize(frame, result_cnn, Size(RetinaWidth, RetinaHeight), INTER_LINEAR);
//         // Get the face
//         Rtn.detect_retinaface(result_cnn, Faces);
//         // Only one face per picture
//         if (Faces.size() == 1) {
//             if (Faces[0].FaceProb > MinFaceThreshold) {
//                 // Get centre aligned image
//                 cv::Mat aligned = Warp.Process(result_cnn, Faces[0]);

//                 std::string Str = imagepath;
//                 n = Str.rfind('/');
//                 Str = Str.erase(0, n + 1);
//                 Str = Str.erase(Str.length() - 4, Str.length() - 1);  // Remove .jpg

//                 imwrite("./img/" + Str + ".jpg", aligned);
//                 cout << "Stored to database : " << Str << endl;
//             }
//         }
//         return 0;
//     }

//     // Loading the faces
//     cv::glob(pattern_jpg, NameFaces);
//     FaceCnt = NameFaces.size();
//     if (FaceCnt == 0) {
//         cout << "No image files[jpg] in database" << endl;
//     } else {
//         cout << "Found " << FaceCnt << " pictures in database." << endl;
//         for (i = 0; i < FaceCnt; i++) {
//             // Convert to landmark vector and store into fc
//             faces = cv::imread(NameFaces[i]);
//             fc1.push_back(ArcFace.GetFeature(faces));
//             // Get a proper name
//             std::string &Str = NameFaces[i];
//             n = Str.rfind('/');
//             Str = Str.erase(0, n + 1);
//             n = Str.find('#');
//             if (n > 0) Str = Str.erase(n, Str.length() - 1);                // Remove # some numbers.jpg
//             else      Str = Str.erase(Str.length() - 4, Str.length() - 1);  // Remove .jpg
//             if (FaceCnt > 1) printf("\rloading: %.2lf%% ", (i * 100.0) / (FaceCnt - 1));
//         }
//         cout << "" << endl;
//         cout << "Loaded " << FaceCnt << " faces in total" << endl;
//     }

//     // RaspiCam or Norton_2.mp4 ?
//      cv::VideoCapture cap("zoom.mp4"); // RaspiCam
//     // cv::VideoCapture cap(0); // Assuming USB camera is the first device (index 0)
//    // cv::VideoCapture cap("rtsp://admin:miet@2024@10.253.0.72"); // Movie
//     if (!cap.isOpened()) {
//         cerr << "ERROR: Unable to open the camera" << endl;
//         return 0;
//     }
//     cout << "Start grabbing, press ESC on TLive window to terminate" << endl;
//     while (1) {
//         cap >> frame;
//         if (frame.empty()) {
//             cerr << "End of movie" << endl;
//             break;
//         }
//         ScaleX = ((float) frame.cols) / RetinaWidth;
//         ScaleY = ((float) frame.rows) / RetinaHeight;

//         // Copy/resize image to result_cnn as input tensor
//         cv::resize(frame, result_cnn, Size(RetinaWidth, RetinaHeight), INTER_LINEAR);

//         Tbegin = chrono::steady_clock::now();

//         #ifdef RETINA
//         Rtn.detect_retinaface(result_cnn, Faces);
//         #else
//         MtCNN.detect(result_cnn, Faces);
//         #endif // RETINA

//         #ifdef RECOGNIZE_FACE
//         // Reset indicators
//         for (i = 0; i < Faces.size(); i++) {
//             Faces[i].NameIndex = -2;    // -2 -> too tiny (may be negative to signal the drawing)
//             Faces[i].Color = 2;
//             Faces[i].NameProb = 0.0;
//             Faces[i].LiveProb = 0.0;
//         }

//         // Process each detected face
//         for (i = 0; i < Faces.size(); i++) {
//             if (Faces[i].FaceProb > MinFaceThreshold) {
//                 // Get centre aligned image
//                 cv::Mat aligned = Warp.Process(result_cnn, Faces[i]);
//                 Faces[i].Angle = Warp.Angle;

//                 // Features of camera image
//                 cv::Mat fc2 = ArcFace.GetFeature(aligned);
//                 // Reset indicators
//                 Faces[i].NameIndex = -1;    // A stranger
//                 Faces[i].Color = 1;

//                 // The similarity score
//                 if (FaceCnt > 0) {
//                     vector<double> score_;
//                     for (size_t c = 0; c < FaceCnt; c++) {
//                         score_.push_back(CosineDistance(fc1[c], fc2));
//                     }
//                     int Pmax = max_element(score_.begin(), score_.end()) - score_.begin();
//                     Faces[i].NameIndex = Pmax;
//                     Faces[i].NameProb = score_[Pmax];
//                     score_.clear();
//                     if (Faces[i].NameProb >= MinFaceThreshold) {
//                         // Recognize a face
//                         if (Faces[i].rect.height < MinHeightFace) {
//                             Faces[i].Color = 2; // Found face in database, but too tiny
//                         } else {
//                             Faces[i].Color = 0; // Found face in database and of good size

//                             // Push emp_code into the queue
//                             std::string emp_code = NameFaces[Faces[i].NameIndex];
//                             emp_queue.push({emp_code, std::time(nullptr)});
//                         }
//                     } else {
//                         Faces[i].NameIndex = -1;    // A stranger
//                         Faces[i].Color = 1;
//                     }
//                 }

//                 // Test if the face is recognized, or should it be added to database
//                 if (Faces[i].NameIndex == -1) {
//                     if (Faces[i].rect.height < MinHeightFace) {
//                         // A stranger with a small face
//                         Faces[i].Color = 2; // Too tiny
//                     }
//                 }
//             }
//         }
//         #endif // RECOGNIZE_FACE

//         Tend = chrono::steady_clock::now();

//         DrawObjects(frame, Faces);

//         // Calculate frame rate
//         f = chrono::duration_cast <chrono::milliseconds> (Tend - Tbegin).count();
//         if (f > 0.0) FPS[((Fcnt++) & 0x0F)] = 1000.0 / f;
//         for (f = 0.0, i = 0; i < 16; i++) { f += FPS[i]; }
//         cv::putText(frame, cv::format("FPS %0.2f", f / 16), cv::Point(10, 20), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(180, 180, 0));

//         // Show output
//         cv::imshow("Jetson Nano - 2014.5 MHz", frame);
//         char esc = cv::waitKey(5);
//         if (esc == 27) break;

//         // Process the queue periodically
//         processQueue(emp_queue, processed_codes, lastRequestTime, curlRequestCount, failedRequests);

//         // If there are failed requests, process them again
//         while (!failedRequests.empty()) {
//             // Push failed requests back to the queue
//             emp_queue.push({failedRequests.front(), std::time(nullptr)});
//             failedRequests.pop();
//         }

//         // Process the queue again
//         processQueue(emp_queue, processed_codes, lastRequestTime, curlRequestCount, failedRequests);
//     }

//     cv::destroyAllWindows();

//     return 0;
// }














































































// // FOR DETECTING AND RECOGNIZING FACES FROM FAR AWAY
// #include <iostream>
// #include <opencv2/opencv.hpp>
// #include "TMtCNN.h"
// #include "TArcface.h"
// #include "TRetina.h"
// #include "TWarp.h"
// #include "TLive.h"
// #include "TBlur.h"
// //----------------------------------------------------------------------------------------
// //
// // Created by markson zhang
// //
// // Edited by Xinghao Chen 2020/7/27
// //
// // Modified by Q-engineering 2020/12/28
// //
// //----------------------------------------------------------------------------------------
// // Build defines
// // comment them to turn a function off
// //----------------------------------------------------------------------------------------
// #define RETINA                  //comment if you want to use MtCNN landmark detection instead
// #define RECOGNIZE_FACE
// #define TEST_LIVING
// #define AUTO_FILL_DATABASE
// #define BLUR_FILTER_STRANGER
// // some diagnostics
// #define SHOW_LEGEND
// #define SHOW_LANDMARKS
// //----------------------------------------------------------------------------------------
// // Adjustable Parameters
// //----------------------------------------------------------------------------------------
// const int   MaxItemsDatabase = 2000;
// const int   MinHeightFace    = 10;
// const float MinFaceThreshold = 0.50;
// const float FaceLiving       = 0.93;
// const double MaxBlur         = -25.0;   //more positive = sharper image
// const double MaxAngle        = 10.0;
// //----------------------------------------------------------------------------------------
// // Some globals
// //----------------------------------------------------------------------------------------
// const int   RetinaWidth      = 500;
// const int   RetinaHeight     = 450;
// float ScaleX, ScaleY;
// vector<std::string> NameFaces;
// //----------------------------------------------------------------------------------------
// using namespace std;
// using namespace cv;
// //----------------------------------------------------------------------------------------
// //  Computing the cosine distance between input feature and ground truth feature
// //----------------------------------------------------------------------------------------
// inline float CosineDistance(const cv::Mat &v1, const cv::Mat &v2)
// {
//    double dot = v1.dot(v2);
//    double denom_v1 = norm(v1);
//    double denom_v2 = norm(v2);
//    return dot / (denom_v1 * denom_v2);
// }
// //----------------------------------------------------------------------------------------
// // painting
// //----------------------------------------------------------------------------------------
// void DrawObjects(cv::Mat &frame, vector<FaceObject> &Faces)
// {
//    for(size_t i=0; i < Faces.size(); i++){
//        FaceObject& obj = Faces[i];

// //----- rectangle around the face -------
//        obj.rect.x *= ScaleX;
//        obj.rect.y *= ScaleY;
//        obj.rect.width *= ScaleX;
//        obj.rect.height*= ScaleY;
//        cv::rectangle(frame, obj.rect, cv::Scalar(0, 255, 0));
// //---------------------------------------

// //----- diagnostic ----------------------
// #ifdef SHOW_LANDMARKS
//        for(int u=0;u<5;u++){
//            obj.landmark[u].x*=ScaleX;
//            obj.landmark[u].y*=ScaleY;
//        }

//        cv::circle(frame, obj.landmark[0], 2, cv::Scalar(0, 255, 255), -1);
//        cv::circle(frame, obj.landmark[1], 2, cv::Scalar(0, 255, 255), -1);
//        cv::circle(frame, obj.landmark[2], 2, cv::Scalar(0, 255, 255), -1);
//        cv::circle(frame, obj.landmark[3], 2, cv::Scalar(0, 255, 255), -1);
//        cv::circle(frame, obj.landmark[4], 2, cv::Scalar(0, 255, 255), -1);
// #endif // SHOW_LANDMARKS
// //---------------------------------------
// #ifdef SHOW_LEGEND
//        cv::putText(frame, cv::format("Angle : %0.1f", obj.Angle),cv::Point(10,40),cv::FONT_HERSHEY_SIMPLEX,0.6, cv::Scalar(180, 180, 0));
//        cv::putText(frame, cv::format("Face prob : %0.4f", obj.FaceProb),cv::Point(10,60),cv::FONT_HERSHEY_SIMPLEX,0.6, cv::Scalar(180, 180, 0));
//        cv::putText(frame, cv::format("Name prob : %0.4f", obj.NameProb),cv::Point(10,80),cv::FONT_HERSHEY_SIMPLEX,0.6, cv::Scalar(180, 180, 0));
// #ifdef TEST_LIVING
//        if(obj.Color==2){
//            //face is too tiny
//            cv::putText(frame, cv::format("Live prob : ??"),cv::Point(10,100),cv::FONT_HERSHEY_SIMPLEX,0.6, cv::Scalar(180, 180, 0));
//        }
//        else{
//            //face is ok
//            cv::putText(frame, cv::format("Live prob : %0.4f", obj.LiveProb),cv::Point(10,100),cv::FONT_HERSHEY_SIMPLEX,0.6, cv::Scalar(180, 180, 0));
//        }
// #endif // TEST_LIVING
// #endif // SHOW_LEGEND
// //----- labels ----------------------------
// #ifdef RECOGNIZE_FACE
//        std::string Str;
//        cv::Scalar color;
//        int  baseLine = 0;

//        switch(obj.Color){
//            case 0 : color = cv::Scalar(255, 255, 255); break;  //default white -> face ok
//            case 1 : color = cv::Scalar( 80, 255, 255); break;  //yellow ->stranger
//            case 2 : color = cv::Scalar(255, 237, 178); break;  //blue -> too tiny
//            case 3 : color = cv::Scalar(127, 127, 255); break;  //red -> fake
//            default: color = cv::Scalar(255, 255, 255);
//        }

//        switch(obj.NameIndex){
//            case -1: Str="Stranger"; break;
//            case -2: Str="too tiny"; break;
//            case -3: Str="Fake !";   break;
//            default: Str=NameFaces[obj.NameIndex];
//        }

//        cv::Size label_size = cv::getTextSize(Str, cv::FONT_HERSHEY_SIMPLEX, 0.6, 1, &baseLine);
//        int x = obj.rect.x;
//        int y = obj.rect.y - label_size.height - baseLine;
//        if(y<0) y = 0;
//        if(x+label_size.width > frame.cols) x=frame.cols-label_size.width;

//        cv::rectangle(frame, cv::Rect(cv::Point(x, y), cv::Size(label_size.width, label_size.height + baseLine)),color, -1);
//        cv::putText(frame, Str, cv::Point(x, y+label_size.height+2),cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 0));
// #endif // RECOGNIZE_FACE
//    }
// }
// //----------------------------------------------------------------------------------------
// // main
// //----------------------------------------------------------------------------------------
// int main(int argc, char **argv)
// {
//    float f;
//    float FPS[16];
//    int   n,Fcnt=0;
//    size_t i;
//    cv::Mat frame;
//    cv::Mat result_cnn;
//    cv::Mat faces;
//    std::vector<FaceObject> Faces;
//    vector<cv::Mat> fc1;
//    string pattern_jpg = "./img/*.jpg";
//    std::string NewItemName;
//    size_t FaceCnt;
//    //the networks
//    TLive Live;
//    TWarp Warp;
//    TMtCNN MtCNN;
//    TArcFace ArcFace;
//    TRetina Rtn(RetinaWidth, RetinaHeight, true);     //have Vulkan support on a Jetson Nano
//    TBlur Blur;
//    //some timing
//    chrono::steady_clock::time_point Tbegin, Tend;

//    Live.LoadModel();

//    for(i=0;i<16;i++) FPS[i]=0.0;

//    //OpenCV Version
//    cout << "OpenCV Version: " << CV_MAJOR_VERSION << "." << CV_MINOR_VERSION << "."
//    << CV_SUBMINOR_VERSION << endl;
//    cout << " " << endl;

// #ifdef RECOGNIZE_FACE
//    cout << "Trying to recognize faces" << endl;
//    cout << " " << endl;
// #ifdef RETINA
//    cout << "Using Retina" << endl;
//    cout << " " << endl;
// #else
//    cout << "Using MtCNN" << endl;
//    cout << " " << endl;
// #endif // RETINA

// #ifdef TEST_LIVING
//    cout << "Test living or fake fave" << endl;
//    cout << " " << endl;
// #endif // TEST_LIVING

// #ifdef AUTO_FILL_DATABASE
//    cout << "Automatic adding strangers to database" << endl;
//    cout << " " << endl;
// #ifdef BLUR_FILTER_STRANGER
//    cout << "Blur filter - only sharp images to database" << endl;
//    cout << " " << endl;
// #endif // BLUR_FILTER_STRANGER
// #endif // AUTO_FILL_DATABASE
// #endif // RECOGNIZE_FACE

//    //if you like to load a picture of a face into the database
//    //give the name of the .jpg image as argument on the command line
//    //without arguments the app will run the .mp4 video or use the camera
//    if(argc>1){
//        const char* imagepath = argv[1];

//        cv::Mat frame = cv::imread(imagepath, 1);
//        if(frame.empty()){
//            fprintf(stderr, "cv::imread %s failed\n", imagepath);
//            return -1;
//        }
//        //extract
//        ScaleX = ((float) frame.cols) / RetinaWidth;
//        ScaleY = ((float) frame.rows) / RetinaHeight;
//        // copy/resize image to result_cnn as input tensor
//        cv::resize(frame, result_cnn, Size(RetinaWidth,RetinaHeight),INTER_LINEAR);
//        //get the face
//        Rtn.detect_retinaface(result_cnn,Faces);
//        //only one face per picture
//        //if(Faces.size()==1){
//        if(1){
//            if(1){
//                //get centre aligned image
//                cv::Mat aligned = Warp.Process(result_cnn,Faces[0]);

//                std::string Str = imagepath;
//                n   = Str.rfind('/');
//                Str = Str.erase(0,n+1);
//                Str = Str.erase(Str.length()-4, Str.length()-1);  //remove .jpg


//                imwrite("./img/"+Str+".jpg", aligned);
//                cout << "Stored to database : " << Str << endl;
//            }
//        }
//        return 0;
//    }


//    //loading the faces
//  cv::glob(pattern_jpg, NameFaces);
//    FaceCnt=NameFaces.size();
//  if(FaceCnt==0) {
//      cout << "No image files[jpg] in database" << endl;
//  }
//  else{
//        cout << "Found "<< FaceCnt << " pictures in database." << endl;
//        for(i=0; i<FaceCnt; i++){
//            //convert to landmark vector and store into fc
//            faces = cv::imread(NameFaces[i]);
//            fc1.push_back(ArcFace.GetFeature(faces));
//            //get a proper name
//            std::string &Str = NameFaces[i];
//            n   = Str.rfind('/');
//            Str = Str.erase(0,n+1);
//            n   = Str.find('#');
//            if(n>0) Str = Str.erase(n,Str.length()-1);                //remove # some numbers.jpg
//            else    Str = Str.erase(Str.length()-4, Str.length()-1);  //remove .jpg
//            if(FaceCnt>1) printf("\rloading: %.2lf%% ",(i*100.0)/(FaceCnt-1));
//        }
//        cout << "" << endl;
//        cout << "Loaded "<<FaceCnt<<" faces in total"<<endl;
//    }

//    // RaspiCam or Norton_2.mp4 ?
//    // cv::VideoCapture cap(0);             //RaspiCam
//    //cv::VideoCapture cap("rtsp://admin:miet@2024@10.253.0.74");   //Movie
//    cv::VideoCapture cap("zoom.mp4");
//    if (!cap.isOpened()) {
//        cerr << "ERROR: Unable to open the camera" << endl;
//        return 0;
//    }
//    cout << "Start grabbing, press ESC on TLive window to terminate" << endl;

//    while(1){
//        cap >> frame;
//        if (frame.empty()) {
//            cerr << "End of movie" << endl;
//            break;
//        }
//        ScaleX = ((float) frame.cols) / RetinaWidth;
//        ScaleY = ((float) frame.rows) / RetinaHeight;

//        // copy/resize image to result_cnn as input tensor
//        cv::resize(frame, result_cnn, Size(RetinaWidth,RetinaHeight),INTER_LINEAR);

//        Tbegin = chrono::steady_clock::now();

// #ifdef RETINA
//        Rtn.detect_retinaface(result_cnn,Faces);
// #else
//        MtCNN.detect(result_cnn,Faces);
// #endif // RETINA

// #ifdef RECOGNIZE_FACE
//        //reset indicators
//        for(i=0;i<Faces.size();i++){
//            cout << "Face " << i << ": NameIndex = " << Faces[i].NameIndex << endl;
//            Faces[i].NameIndex = -2;    //-2 -> too tiny (may be negative to signal the drawing)
//            Faces[i].Color     =  2;
//            Faces[i].NameProb  = 0.0;
//            Faces[i].LiveProb  = 0.0;
//        }
//        //run through the faces only when you got one face.
//        //more faces (if large enough) are not a problem
//        //in this app with an input image of 324x240, they become too tiny
//        if(Faces.size()==1){
//            //looks stupid, running through a loop of size 1
//            //however, for your convenience using [i]
//            for(i=0;i<Faces.size();i++){
//                if(Faces[i].FaceProb>MinFaceThreshold){
//                    //get centre aligned image
//                    cv::Mat aligned = Warp.Process(result_cnn,Faces[i]);
//                    Faces[i].Angle  = Warp.Angle;
//                    //features of camera image
//                    cv::Mat fc2 = ArcFace.GetFeature(aligned);
//                    //reset indicators
//                    Faces[i].NameIndex = -1;    //a stranger
//                    Faces[i].Color     =  1;
//                    //the similarity score
//                    if(FaceCnt>0){
//                        vector<double> score_;
//                        for(size_t c=0;c<FaceCnt;c++) score_.push_back(CosineDistance(fc1[c], fc2));
//                        int Pmax = max_element(score_.begin(),score_.end()) - score_.begin();
//                        Faces[i].NameIndex = Pmax;
//                        Faces[i].NameProb  = score_[Pmax];
//                        score_.clear();
//                        if(Faces[i].NameProb >= MinFaceThreshold){
//                            //recognize a face
//                            if(Faces[i].rect.height < MinHeightFace){
//                                Faces[i].Color = 2; //found face in database, but too tiny
//                            }
//                            else{
//                                Faces[i].Color = 0; //found face in database and of good size
// #ifdef TEST_LIVING
//                                //test fake face
//                                float x1 = Faces[i].rect.x;
//                                float y1 = Faces[i].rect.y;
//                                float x2 = Faces[i].rect.width+x1;
//                                float y2 = Faces[i].rect.height+y1;
//                                struct LiveFaceBox LiveBox={x1,y1,x2,y2};

//                                Faces[i].LiveProb=Live.Detect(result_cnn,LiveBox);
//                                if(Faces[i].LiveProb<=FaceLiving){
//                                    Faces[i].Color     =  3; //fake
//                                    Faces[i].NameIndex = -3;
//                                }
// #endif // TEST_LIVING
//                            }
//                        }
//                        else{
//                            Faces[i].NameIndex = -1;    //a stranger
//                            Faces[i].Color     =  1;
//                        }
//                    }

//                    //test if the face is recognized, or should it be added to database
//                   //test if the face is recognized, or should it be added to database
// if(Faces[i].NameIndex == 1090519040 || Faces[i].NameIndex == -2 || Faces[i].NameIndex == -3){
//     // A stranger, tiny face, or fake face - add them to the database
//     cout << "Detected face with NameIndex: " << Faces[i].NameIndex << endl;  // Debugging statement
// #ifdef AUTO_FILL_DATABASE
//     if(FaceCnt >= MaxItemsDatabase){
//         cout << "Database full !" << endl;
//     }
//     else{
//         double blur = 0.0;
// #ifdef BLUR_FILTER_STRANGER
//         blur = Blur.Execute(aligned);
// #endif // BLUR_FILTER_STRANGER

//         // Log values of blur and angle for debugging purposes
//         cout << "Warp.Angle: " << Warp.Angle << ", MaxAngle: " << MaxAngle << endl;
//         cout << "Blur: " << blur << ", MaxBlur: " << MaxBlur << endl;

//         // Ensure that all faces are added regardless of size, but still apply angle/blur checks if enabled.
//         if(Warp.Angle <= MaxAngle && blur >= MaxBlur){
//             cout << "Adding face to database." << endl;  // Debugging statement
//             FaceCnt++;
//             NewItemName = format("person %i", (int)FaceCnt);
//             fc1.push_back(fc2);   // Add feature vector for face recognition
//             NameFaces.push_back(NewItemName);  // Store the name for this new face
//             imwrite("./img/" + NewItemName + ".jpg", aligned);  // Save the aligned image to the dataset
//             cout << "Stored to database: " << NewItemName << endl;
//         } else {
//             cout << "Skipped adding face due to blur/angle criteria." << endl;
//         }
//     }
// #endif // AUTO_FILL_DATABASE
// }



//                }
//            }
//        }
// #endif // RECOGNIZE_FACE

//        Tend = chrono::steady_clock::now();

//        DrawObjects(frame, Faces);

//        //calculate frame rate
//        f = chrono::duration_cast <chrono::milliseconds> (Tend - Tbegin).count();
//        if(f>0.0) FPS[((Fcnt++)&0x0F)]=1000.0/f;
//        for(f=0.0, i=0;i<16;i++){ f+=FPS[i]; }
//        cv::putText(frame, cv::format("FPS %0.2f", f/16),cv::Point(10,20),cv::FONT_HERSHEY_SIMPLEX,0.6, cv::Scalar(180, 180, 0));

//        //show output
//        cv::imshow("Jetson Nano - 2014.5 MHz", frame);
//        char esc = cv::waitKey(5);
//        if(esc == 27) break;
//    }

//    cv::destroyAllWindows();

//    return 0;
// }
























































#include <iostream>
#include <opencv2/opencv.hpp>
#include "TMtCNN.h"
#include "TArcface.h"
#include "TRetina.h"
#include "TWarp.h"
#include "TLive.h"
#include "TBlur.h"

#include <thread> // (added by me)
#include <chrono>
#include <map>

//----------------------------------------------------------------------------------------
//
// Created by Bhavan Prakash
//
//
//----------------------------------------------------------------------------------------
// Build defines
// comment them to turn a function off
//----------------------------------------------------------------------------------------
#define RETINA                  //comment if you want to use MtCNN landmark detection instead
#define RECOGNIZE_FACE
#define TEST_LIVING
#define AUTO_FILL_DATABASE
#define BLUR_FILTER_STRANGER
// some diagnostics
#define SHOW_LEGEND
#define SHOW_LANDMARKS
//----------------------------------------------------------------------------------------
// Adjustable Parameters
//----------------------------------------------------------------------------------------
const int   MaxItemsDatabase = 2000;
const int   MinHeightFace    = 90;
const float MinFaceThreshold = 0.30;
const float FaceLiving       = 0.93;
const double MaxBlur         = -25.0;   //more positive = sharper image
const double MaxAngle        = 10.0;
//----------------------------------------------------------------------------------------
// Some globals
//----------------------------------------------------------------------------------------
const int   RetinaWidth      = 600;
const int   RetinaHeight     = 550;
float ScaleX, ScaleY;
vector<std::string> NameFaces;
//----------------------------------------------------------------------------------------
using namespace std;
using namespace cv;
//----------------------------------------------------------------------------------------
//  Computing the cosine distance between input feature and ground truth feature
//----------------------------------------------------------------------------------------
inline float CosineDistance(const cv::Mat &v1, const cv::Mat &v2)
{
    double dot = v1.dot(v2);
    double denom_v1 = norm(v1);
    double denom_v2 = norm(v2);
    return dot / (denom_v1 * denom_v2);
}
//----------------------------------------------------------------------------------------
// painting
//----------------------------------------------------------------------------------------
void DrawObjects(cv::Mat &frame, vector<FaceObject> &Faces)
{
    for(size_t i=0; i < Faces.size(); i++){
        FaceObject& obj = Faces[i];

//----- rectangle around the face -------
        obj.rect.x *= ScaleX;
        obj.rect.y *= ScaleY;
        obj.rect.width *= ScaleX;
        obj.rect.height*= ScaleY;
        cv::rectangle(frame, obj.rect, cv::Scalar(0, 255, 0));
//---------------------------------------

//----- diagnostic ----------------------
#ifdef SHOW_LANDMARKS
        for(int u=0;u<5;u++){
            obj.landmark[u].x*=ScaleX;
            obj.landmark[u].y*=ScaleY;
        }

        cv::circle(frame, obj.landmark[0], 2, cv::Scalar(0, 255, 255), -1);
        cv::circle(frame, obj.landmark[1], 2, cv::Scalar(0, 255, 255), -1);
        cv::circle(frame, obj.landmark[2], 2, cv::Scalar(0, 255, 255), -1);
        cv::circle(frame, obj.landmark[3], 2, cv::Scalar(0, 255, 255), -1);
        cv::circle(frame, obj.landmark[4], 2, cv::Scalar(0, 255, 255), -1);
#endif // SHOW_LANDMARKS
//---------------------------------------
#ifdef SHOW_LEGEND
        cv::putText(frame, cv::format("Angle : %0.1f", obj.Angle),cv::Point(10,40),cv::FONT_HERSHEY_SIMPLEX,0.6, cv::Scalar(180, 180, 0));
        cv::putText(frame, cv::format("Face prob : %0.4f", obj.FaceProb),cv::Point(10,60),cv::FONT_HERSHEY_SIMPLEX,0.6, cv::Scalar(180, 180, 0));
        cv::putText(frame, cv::format("Name prob : %0.4f", obj.NameProb),cv::Point(10,80),cv::FONT_HERSHEY_SIMPLEX,0.6, cv::Scalar(180, 180, 0));
#ifdef TEST_LIVING
        if(obj.Color==2){
            //face is too tiny
            cv::putText(frame, cv::format("Live prob : ??"),cv::Point(10,100),cv::FONT_HERSHEY_SIMPLEX,0.6, cv::Scalar(180, 180, 0));
        }
        else{
            //face is ok
            cv::putText(frame, cv::format("Live prob : %0.4f", obj.LiveProb),cv::Point(10,100),cv::FONT_HERSHEY_SIMPLEX,0.6, cv::Scalar(180, 180, 0));
        }
#endif // TEST_LIVING
#endif // SHOW_LEGEND
//----- labels ----------------------------
#ifdef RECOGNIZE_FACE
        std::string Str;
        cv::Scalar color;
        int  baseLine = 0;

        switch(obj.Color){
            case 0 : color = cv::Scalar(255, 255, 255); break;  //default white -> face ok
            case 1 : color = cv::Scalar( 80, 255, 255); break;  //yellow ->stranger
            case 2 : color = cv::Scalar(255, 237, 178); break;  //blue -> too tiny
            case 3 : color = cv::Scalar(127, 127, 255); break;  //red -> fake
            default: color = cv::Scalar(255, 255, 255);
        }

        switch(obj.NameIndex){
            case -1: Str="Stranger"; break;
            case -2: Str="too tiny"; break;
            case -3: Str="Fake !";   break;
            default: Str=NameFaces[obj.NameIndex];
        }

        cv::Size label_size = cv::getTextSize(Str, cv::FONT_HERSHEY_SIMPLEX, 0.6, 1, &baseLine);
        int x = obj.rect.x;
        int y = obj.rect.y - label_size.height - baseLine;
        if(y<0) y = 0;
        if(x+label_size.width > frame.cols) x=frame.cols-label_size.width;

        cv::rectangle(frame, cv::Rect(cv::Point(x, y), cv::Size(label_size.width, label_size.height + baseLine)),color, -1);
        cv::putText(frame, Str, cv::Point(x, y+label_size.height+2),cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 0));
#endif // RECOGNIZE_FACE
    }
}

int main(int argc, char **argv)
{
    float f;
    float FPS[16];
    int   n,Fcnt=0;
    size_t i;
    cv::Mat frame;
    cv::Mat result_cnn;
    cv::Mat faces;
    std::vector<FaceObject> Faces;
    vector<cv::Mat> fc1;
    string pattern_jpg = "./img/*.jpg";
    std::string NewItemName;
    size_t FaceCnt;
    std::set<int> addedFaces; // Set to keep track of added faces
    //the networks
    TLive Live;
    TWarp Warp;
    TMtCNN MtCNN;
    TArcFace ArcFace;
    TRetina Rtn(RetinaWidth, RetinaHeight, true);     //have Vulkan support on a Jetson Nano
    TBlur Blur;
    //some timing
    chrono::steady_clock::time_point Tbegin, Tend;

    Live.LoadModel();

    for(i=0;i<16;i++) FPS[i]=0.0;

    //OpenCV Version
    cout << "OpenCV Version: " << CV_MAJOR_VERSION << "." << CV_MINOR_VERSION << "."
    << CV_SUBMINOR_VERSION << endl;
    cout << " " << endl;

#ifdef RECOGNIZE_FACE
    cout << "Trying to recognize faces" << endl;
    cout << " " << endl;
#ifdef RETINA
    cout << "Using Retina" << endl;
    cout << " " << endl;
#else
    cout << "Using MtCNN" << endl;
    cout << " " << endl;
#endif // RETINA

#ifdef TEST_LIVING
    cout << "Test living or fake face" << endl;
    cout << " " << endl;
#endif // TEST_LIVING

#ifdef AUTO_FILL_DATABASE
    cout << "Automatic adding strangers to database" << endl;
    cout << " " << endl;
#ifdef BLUR_FILTER_STRANGER
    cout << "Blur filter - only sharp images to database" << endl;
    cout << " " << endl;
#endif // BLUR_FILTER_STRANGER
#endif // AUTO_FILL_DATABASE
#endif // RECOGNIZE_FACE

    //if you like to load a picture of a face into the database
    //give the name of the .jpg image as argument on the command line
    //without arguments the app will run the .mp4 video or use the camera
    if(argc>1){
        const char* imagepath = argv[1];

        cv::Mat frame = cv::imread(imagepath, 1);
        if(frame.empty()){
            fprintf(stderr, "cv::imread %s failed\n", imagepath);
            return -1;
        }
        //extract
        ScaleX = ((float) frame.cols) / RetinaWidth;
        ScaleY = ((float) frame.rows) / RetinaHeight;
        // copy/resize image to result_cnn as input tensor
        cv::resize(frame, result_cnn, Size(RetinaWidth,RetinaHeight),INTER_LINEAR);
        //get the face
        Rtn.detect_retinaface(result_cnn,Faces);
        //only one face per picture
        if(Faces.size()==1){
            if(Faces[0].FaceProb>MinFaceThreshold){
                //get centre aligned image
                cv::Mat aligned = Warp.Process(result_cnn,Faces[0]);

                std::string Str = imagepath;
                n   = Str.rfind('/');
                Str = Str.erase(0,n+1);
                Str = Str.erase(Str.length()-4, Str.length()-1);  //remove .jpg

                imwrite("./training/"+Str+".jpg", aligned);
                cout << "Stored to database : " << Str << endl;
            }
        }
        return 0;
    }

    //loading the faces
    cv::glob(pattern_jpg, NameFaces);
    FaceCnt=NameFaces.size();
    if(FaceCnt==0) {
        cout << "No image files[jpg] in database" << endl;
    }
    else{
        cout << "Found "<< FaceCnt << " pictures in database." << endl;
        for(i=0; i<FaceCnt; i++){
            //convert to landmark vector and store into fc
            faces = cv::imread(NameFaces[i]);
            fc1.push_back(ArcFace.GetFeature(faces));
            //get a proper name
            std::string &Str = NameFaces[i];
            n   = Str.rfind('/');
            Str = Str.erase(0,n+1);
            n   = Str.find('#');
            if(n>0) Str = Str.erase(n,Str.length()-1);                //remove # some numbers.jpg
            else    Str = Str.erase(Str.length()-4, Str.length()-1);  //remove .jpg
            if(FaceCnt>1) printf("\rloading: %.2lf%% ",(i*100.0)/(FaceCnt-1));
        }
        cout << "" << endl;
        cout << "Loaded "<<FaceCnt<<" faces in total"<<endl;
    }

    // RaspiCam or Norton_2.mp4 ?
    // cv::VideoCapture cap(0);             //RaspiCam
    //cv::VideoCapture cap("Norton_A.mp4");   //Movie
    cv::VideoCapture cap("rtsp://admin:miet@2024@10.253.0.76"); // Movie
    if (!cap.isOpened()) {
        cerr << "ERROR: Unable to open the camera" << endl;
        return 0;
    }
    cout << "Start grabbing, press ESC on TLive window to terminate" << endl;

    while(1){
        cap >> frame;
        if (frame.empty()) {
            cerr << "End of movie" << endl;
            break;
        }
        ScaleX = ((float) frame.cols) / RetinaWidth;
        ScaleY = ((float) frame.rows) / RetinaHeight;

        // copy/resize image to result_cnn as input tensor
        cv::resize(frame, result_cnn, Size(RetinaWidth,RetinaHeight),INTER_LINEAR);

        Tbegin = chrono::steady_clock::now();

#ifdef RETINA
        Rtn.detect_retinaface(result_cnn,Faces);
#else
        MtCNN.detect(result_cnn,Faces);
#endif // RETINA

#ifdef RECOGNIZE_FACE
        //reset indicators
        for(i=0;i<Faces.size();i++){
            Faces[i].NameIndex = -2;    //-2 -> too tiny (may be negative to signal the drawing)
            Faces[i].Color     =  2;
            Faces[i].NameProb  = 0.0;
            Faces[i].LiveProb  = 0.0;
        }

        // Keep track of faces added in the current frame
        addedFaces.clear();

        //run through the faces
        for(i=0;i<Faces.size();i++){
            //get centre aligned image
            cv::Mat aligned = Warp.Process(result_cnn,Faces[i]);
            Faces[i].Angle  = Warp.Angle;
            //features of camera image
            cv::Mat fc2 = ArcFace.GetFeature(aligned);

            Faces[i].NameIndex = -1;    //a stranger
            Faces[i].Color     =  1;

            // Ensure only one photo per person
            int personID = Faces[i].rect.x; // You may use a more reliable ID
            if(addedFaces.find(personID) == addedFaces.end()){
                if(FaceCnt>=MaxItemsDatabase){
                    cout << "Database full !" << endl;
                }
//                 else{
//                     double blur=0.0;
// #ifdef BLUR_FILTER_STRANGER
//                     blur=Blur.Execute(aligned);
// #endif // BLUR_FILTER_STRANGER
//                     if(Warp.Angle<=MaxAngle && blur>=MaxBlur){
//                         FaceCnt++;
//                         NewItemName = format("A118#%i",(int)FaceCnt);
//                         fc1.push_back(fc2);
//                         NameFaces.push_back(NewItemName);
//                         imwrite("./A118/"+NewItemName+".jpg", aligned);
//                         cout << "Stored to database : " << NewItemName << endl;
//                         addedFaces.insert(personID); // Mark this face as added
//                     }
//                 }

                else{
                    double blur=0.0;
#ifdef BLUR_FILTER_STRANGER
                    blur=Blur.Execute(aligned);
#endif // BLUR_FILTER_STRANGER
                    if(Warp.Angle<=MaxAngle && blur>=MaxBlur){
                        FaceCnt++;
                        NewItemName = format("41110#%i",(int)FaceCnt);
                        fc1.push_back(fc2);
                        NameFaces.push_back(NewItemName);
                        imwrite("./41110/"+NewItemName+".jpg", aligned);
                        cout << "Stored to database : " << NewItemName << endl;
                        addedFaces.insert(personID); // Mark this face as added
                    }
                }


            }
        }
#endif // RECOGNIZE_FACE

        Tend = chrono::steady_clock::now();

        DrawObjects(frame, Faces);

        //calculate frame rate
        f = chrono::duration_cast <chrono::milliseconds> (Tend - Tbegin).count();
        if(f>0.0) FPS[((Fcnt++)&0x0F)]=1000.0/f;
        for(f=0.0, i=0;i<16;i++){ f+=FPS[i]; }
        cv::putText(frame, cv::format("FPS %0.2f", f/16),cv::Point(10,20),cv::FONT_HERSHEY_SIMPLEX,0.6, cv::Scalar(180, 180, 0));

        //show output
        cv::imshow("Jetson Nano - 2014.5 MHz", frame);
        char esc = cv::waitKey(5);
        if(esc == 27) break;
    }

    cv::destroyAllWindows();

    return 0;
}


//    WITH DELAY( BUT THE VIDEO INPUT IS DELAYING INSTEAD OF IMAGE PROCESSING )
// std::this_thread::sleep_for(std::chrono::seconds(5));


// void saveImageWithDelay(const std::string& filename, std::shared_ptr<cv::Mat> image) {
//     std::this_thread::sleep_for(std::chrono::seconds(5));
//     if (!image->empty()) {
//         cv::imwrite(filename, *image);
//         std::cout << "Stored to database: " << filename << std::endl;
//     } else {
//         std::cerr << "Error: Image is empty, cannot save to database." << std::endl;
//     }
// }

// int main(int argc, char **argv)
// {
//     float f;
//     float FPS[16];
//     int   n,Fcnt=0;
//     size_t i;
//     cv::Mat frame;
//     cv::Mat result_cnn;
//     cv::Mat faces;
//     std::vector<FaceObject> Faces;
//     vector<cv::Mat> fc1;
//     string pattern_jpg = "./img/*.jpg";
//     std::string NewItemName;
//     size_t FaceCnt;
//     std::set<int> addedFaces; // Set to keep track of added faces
//     //the networks
//     TLive Live;
//     TWarp Warp;
//     TMtCNN MtCNN;
//     TArcFace ArcFace;
//     TRetina Rtn(RetinaWidth, RetinaHeight, true);     //have Vulkan support on a Jetson Nano
//     TBlur Blur;
//     //some timing
//     chrono::steady_clock::time_point Tbegin, Tend;

//     Live.LoadModel();

//     for(i=0;i<16;i++) FPS[i]=0.0;

//     //OpenCV Version
//     cout << "OpenCV Version: " << CV_MAJOR_VERSION << "." << CV_MINOR_VERSION << "."
//     << CV_SUBMINOR_VERSION << endl;
//     cout << " " << endl;

// #ifdef RECOGNIZE_FACE
//     cout << "Trying to recognize faces" << endl;
//     cout << " " << endl;
// #ifdef RETINA
//     cout << "Using Retina" << endl;
//     cout << " " << endl;
// #else
//     cout << "Using MtCNN" << endl;
//     cout << " " << endl;
// #endif // RETINA

// #ifdef TEST_LIVING
//     cout << "Test living or fake face" << endl;
//     cout << " " << endl;
// #endif // TEST_LIVING

// #ifdef AUTO_FILL_DATABASE
//     cout << "Automatic adding strangers to database" << endl;
//     cout << " " << endl;
// #ifdef BLUR_FILTER_STRANGER
//     cout << "Blur filter - only sharp images to database" << endl;
//     cout << " " << endl;
// #endif // BLUR_FILTER_STRANGER
// #endif // AUTO_FILL_DATABASE
// #endif // RECOGNIZE_FACE

//     //if you like to load a picture of a face into the database
//     //give the name of the .jpg image as argument on the command line
//     //without arguments the app will run the .mp4 video or use the camera
//     if(argc>1){
//         const char* imagepath = argv[1];

//         cv::Mat frame = cv::imread(imagepath, 1);
//         if(frame.empty()){
//             fprintf(stderr, "cv::imread %s failed\n", imagepath);
//             return -1;
//         }
//         //extract
//         ScaleX = ((float) frame.cols) / RetinaWidth;
//         ScaleY = ((float) frame.rows) / RetinaHeight;
//         // copy/resize image to result_cnn as input tensor
//         cv::resize(frame, result_cnn, Size(RetinaWidth,RetinaHeight),INTER_LINEAR);
//         //get the face
//         Rtn.detect_retinaface(result_cnn,Faces);
//         //only one face per picture
//         if(Faces.size()==1){
//             if(Faces[0].FaceProb>MinFaceThreshold){
//                 //get centre aligned image
//                 cv::Mat aligned = Warp.Process(result_cnn,Faces[0]);

//                 std::string Str = imagepath;
//                 n   = Str.rfind('/');
//                 Str = Str.erase(0,n+1);
//                 Str = Str.erase(Str.length()-4, Str.length()-1);  //remove .jpg

//                 imwrite("./img/"+Str+".jpg", aligned);
//                 cout << "Stored to database : " << Str << endl;
//             }
//         }
//         return 0;
//     }

//     //loading the faces
//     cv::glob(pattern_jpg, NameFaces);
//     FaceCnt=NameFaces.size();
//     if(FaceCnt==0) {
//         cout << "No image files[jpg] in database" << endl;
//     }
//     else{
//         cout << "Found "<< FaceCnt << " pictures in database." << endl;
//         for(i=0; i<FaceCnt; i++){
//             //convert to landmark vector and store into fc
//             faces = cv::imread(NameFaces[i]);
//             fc1.push_back(ArcFace.GetFeature(faces));
//             //get a proper name
//             std::string &Str = NameFaces[i];
//             n   = Str.rfind('/');
//             Str = Str.erase(0,n+1);
//             n   = Str.find('#');
//             if(n>0) Str = Str.erase(n,Str.length()-1);                //remove # some numbers.jpg
//             else    Str = Str.erase(Str.length()-4, Str.length()-1);  //remove .jpg
//             if(FaceCnt>1) printf("\rloading: %.2lf%% ",(i*100.0)/(FaceCnt-1));
//         }
//         cout << "" << endl;
//         cout << "Loaded "<<FaceCnt<<" faces in total"<<endl;
//     }

//     // RaspiCam or Norton_2.mp4 ?
//     // cv::VideoCapture cap(0);             //RaspiCam
//     cv::VideoCapture cap("Norton_A.mp4");   //Movie
//     if (!cap.isOpened()) {
//         cerr << "ERROR: Unable to open the camera" << endl;
//         return 0;
//     }
//     cout << "Start grabbing, press ESC on TLive window to terminate" << endl;

//     while(1){
//         cap >> frame;
//         if (frame.empty()) {
//             cerr << "End of movie" << endl;
//             break;
//         }
//         ScaleX = ((float) frame.cols) / RetinaWidth;
//         ScaleY = ((float) frame.rows) / RetinaHeight;

//         // copy/resize image to result_cnn as input tensor
//         cv::resize(frame, result_cnn, Size(RetinaWidth, RetinaHeight), INTER_LINEAR);

//         Tbegin = chrono::steady_clock::now();

// #ifdef RETINA
//         Rtn.detect_retinaface(result_cnn,Faces);
// #else
//         MtCNN.detect(result_cnn,Faces);
// #endif // RETINA

// #ifdef RECOGNIZE_FACE
//         for(i=0; i<Faces.size(); i++){
//             Faces[i].NameIndex = -1;    // Mark as a stranger
//             Faces[i].Color     =  1;

//             cv::Mat aligned = Warp.Process(result_cnn, Faces[i]);
//             Faces[i].Angle = Warp.Angle;
//             cv::Mat fc2 = ArcFace.GetFeature(aligned);

//             if(FaceCnt >= MaxItemsDatabase) {
//                 cout << "Database full!" << endl;
//                 break;
//             } else {
//                 double blur = 0.0;
// #ifdef BLUR_FILTER_STRANGER
//     blur = Blur.Execute(aligned);
// #endif // BLUR_FILTER_STRANGER

//     if (Warp.Angle <= MaxAngle && blur >= MaxBlur) {
//         FaceCnt++;
//         NewItemName = format("person %i", (int)FaceCnt);
//         fc1.push_back(fc2);
//         NameFaces.push_back(NewItemName);

//         // Use std::shared_ptr to manage image lifetime across threads
//         auto imagePtr = std::make_shared<cv::Mat>(aligned);

//         // Start a separate thread for saving the image
//         std::thread(saveImageWithDelay, "./img/" + NewItemName + ".jpg", imagePtr).detach();
//     }
//             }
//         }
// #endif // RECOGNIZE_FACE

//         Tend = chrono::steady_clock::now();

//         DrawObjects(frame, Faces);

//         // Calculate frame rate
//         f = chrono::duration_cast <chrono::milliseconds> (Tend - Tbegin).count();
//         if(f > 0.0) FPS[((Fcnt++) & 0x0F)] = 1000.0 / f;
//         for(f = 0.0, i = 0; i < 16; i++) { f += FPS[i]; }
//         cv::putText(frame, cv::format("FPS %0.2f", f / 16), cv::Point(10, 20), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(180, 180, 0));

//         // Show output
//         cv::imshow("Jetson Nano - 2014.5 MHz", frame);
//         char esc = cv::waitKey(5);
//         if(esc == 27) break;
//     }

//     cv::destroyAllWindows();

//     return 0;
// }
