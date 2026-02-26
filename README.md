# Smart Real-Time Face Recognition & Attendance System  
![output image](assets/system_preview.png)

## A multi-mode face recognition and smart attendance system for edge devices.

[![License](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)<br/>

This C++ application recognizes a person from a database of thousands of faces and can automatically log attendance using a backend API.  
It is optimized for **Jetson Nano** and Linux-based edge devices, but can easily be ported to other platforms.

First, faces and landmarks are detected by **RetinaFace** (or MTCNN).  
Next, the database is scanned using **ArcFace embeddings** and cosine similarity.  
Finally, optional **anti-spoofing**, **blur filtering**, and **angle filtering** ensure reliable and secure recognition.

The system supports:

- 🏢 Real-time attendance logging  
- 🧪 Automatic database expansion  
- 🎯 Distance-aware threshold calibration  
- 🔒 Anti-spoofing and quality control  

---

------------

## System Architecture.

## System Architecture

```
Face-Recognition/
│
├── img/                  (Face database)
├── models/               (Deep learning models)
├── src/
│   ├── main.cpp          (Production attendance mode)
│   ├── main_test.cpp     (Testing & auto-fill mode)
│   ├── main_training.cpp (Threshold tuning mode)
│   ├── TRetina.cpp       (RetinaFace detector)
│   ├── TMtCNN.cpp        (MTCNN detector)
│   ├── TArcface.cpp      (ArcFace embedding extractor)
│   ├── TWarp.cpp         (Face alignment)
│   ├── TLive.cpp         (Anti-spoofing / liveness)
│   ├── TBlur.cpp         (Blur detection)
│
└── include/              (Header files)
```


---

## How it works.

Each frame follows this pipeline:

1. **Frame Capture**
   - USB camera / RTSP IP camera
   - Real-time video stream

2. **Face Detection**
   - RetinaFace (default)
   - MTCNN (optional)

3. **Face Alignment**
   - Landmark-based warping
   - Normalized frontal face

4. **Feature Extraction**
   - ArcFace deep embeddings

5. **Similarity Matching**
   - Cosine similarity with database

6. **Quality & Security Checks**
   - Minimum face size
   - Blur filtering
   - Face angle filtering
   - Optional anti-spoofing

7. **Decision & Attendance Logging**
   - Threshold validation
   - API POST request
   - Duplicate prevention logic

---

------------

## Anti-Spoofing.

The system supports **Face Anti-Spoofing** using the `TLive` module.

This detects whether the face is:

- A real live person  
- A printed photograph  
- A mobile phone display  
- A mask or spoof attempt  

When enabled:

#define TEST_LIVING


Only faces with sufficient liveness probability are accepted.

Disabling spoofing improves FPS but reduces security.

---

------------

## Quality Control.

To maintain database accuracy, several filters are used:

### Blur Filter
Only sharp images are added to the database.

#define BLUR_FILTER_STRANGER


Prevents:
- Motion blur faces
- Low-quality samples
- Database corruption

---

### Face Angle Filtering
Faces must be within an acceptable rotation angle.

Prevents:
- Profile faces
- Extreme head tilt
- Partial face captures

---

### Minimum Face Size Threshold
Small faces (far distance) require higher probability to be accepted.

Example:

| Face Size | Probability | Action |
|------------|------------|--------|
| Large | ≥ 0.55 | Accept |
| Small | ≥ 0.60 | Accept |
| Small | < 0.60 | Reject |

This reduces false positives at long distances.

---

------------

## Modes.

### 1️⃣ Production Mode – `main.cpp`

Full smart attendance system.

Features:
- Multithreaded producer–consumer architecture
- Queue-based processing
- 30-minute duplicate prevention
- API integration
- Failed request retry
- Thread-safe attendance logging

Producer:
Detect → Recognize → Push emp_code to queue


Consumer:
Wait → Send API request → Update timestamp


---

### 2️⃣ Testing Mode – `main_test.cpp`

Used to:
- Expand database automatically
- Measure FPS
- Visualize landmarks
- Validate detection stability

Strangers are added automatically if quality checks pass.

---

### 3️⃣ Threshold Calibration Mode – `main_training.cpp`

Used for:
- Distance-based probability tuning
- Recognition threshold experimentation
- False positive reduction analysis

Designed for scientific calibration before deployment.

---

------------

## Attendance API.

POST request format:

apikey=XXXXX
atype=Check-in
emp_code=EMP123


Security mechanisms:

- Timestamp tracking
- 30-minute rate limiting
- Duplicate prevention
- Retry queue for failed requests

Ensures stable real-time logging.

---

------------

## Deployment Target.

Designed for:

- 🖥 Jetson Nano  
- 🐧 Ubuntu Linux  
- 🎥 RTSP IP Cameras  
- 🏫 Campus-level attendance systems  

Optimized for:

- Edge AI deployment  
- Low-latency processing  
- Real-time recognition  
- Scalable database size (2000+ faces)  

The system can also be ported to:

- Raspberry Pi (reduced FPS)
- x86 Linux systems
- Embedded edge devices

---

------------

## Performance.

Typical processing per frame (Jetson Nano @ 1479 MHz):

| Module | Approx Time |
|--------|------------|
| RetinaFace | ~15–19 ms |
| ArcFace | +17–21 ms |
| Anti-Spoofing | +25–37 ms |

Actual performance depends on:
- Resolution
- Number of faces
- Camera quality
- Spoofing enabled/disabled

---

------------

## Camera Configuration.

WebCam:

cv::VideoCapture cap(0);


RTSP:

cv::VideoCapture cap("rtsp://user:password@ip");


Video file:

cv::VideoCapture cap("video.mp4");


---

------------

## Why this project is strong.

- Modular multi-mode architecture  
- Real-time edge deployment  
- Multithreaded system design  
- Distance-aware threshold engineering  
- Integrated backend attendance system  
- Anti-spoofing & quality validation  
- Production-ready C++ implementation  

This is not just face recognition —  
it is a deployable real-world AI system.

---
