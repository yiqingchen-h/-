#!/usr/bin/env python3
# encoding: utf-8

import math
import time
import cv2 as cv
import numpy as np
import mediapipe as mp
import libcamera
from picamera2 import Picamera2

try:
    import serial
except ImportError:
    serial = None


class handDetector:
    def __init__(self, mode=False, maxHands=1, detectorCon=0.5, trackCon=0.5):
        self.tipIds = [4, 8, 12, 16, 20]
        self.mpHand = mp.solutions.hands
        self.mpDraw = mp.solutions.drawing_utils
        self.hands = self.mpHand.Hands(
            static_image_mode=mode,
            max_num_hands=maxHands,
            min_detection_confidence=detectorCon,
            min_tracking_confidence=trackCon
        )

        self.lmList = []
        self.lmDrawSpec = mp.solutions.drawing_utils.DrawingSpec(
            color=(0, 0, 255), thickness=-1, circle_radius=6
        )
        self.drawSpec = mp.solutions.drawing_utils.DrawingSpec(
            color=(0, 255, 0), thickness=2, circle_radius=2
        )

    def get_dist(self, point1, point2):
        x1, y1 = point1
        x2, y2 = point2
        return math.sqrt((y1 - y2) ** 2 + (x1 - x2) ** 2)

    def calc_angle(self, pt1, pt2, pt3):
        point1 = self.lmList[pt1][1], self.lmList[pt1][2]
        point2 = self.lmList[pt2][1], self.lmList[pt2][2]
        point3 = self.lmList[pt3][1], self.lmList[pt3][2]

        a = self.get_dist(point1, point2)
        b = self.get_dist(point2, point3)
        c = self.get_dist(point1, point3)

        try:
            radian = math.acos((a * a + b * b - c * c) / (2 * a * b))
            angle = radian / math.pi * 180
        except:
            angle = 0

        return abs(angle)

    def findHands(self, frame, draw=True):
        self.lmList = []
        img = np.zeros(frame.shape, np.uint8)
        img_RGB = cv.cvtColor(frame, cv.COLOR_BGR2RGB)
        self.results = self.hands.process(img_RGB)

        if self.results.multi_hand_landmarks:
            # 这里只取一只手
            hand_landmarks = self.results.multi_hand_landmarks[0]

            if draw:
                self.mpDraw.draw_landmarks(
                    frame,
                    hand_landmarks,
                    self.mpHand.HAND_CONNECTIONS,
                    self.lmDrawSpec,
                    self.drawSpec
                )

            self.mpDraw.draw_landmarks(
                img,
                hand_landmarks,
                self.mpHand.HAND_CONNECTIONS,
                self.lmDrawSpec,
                self.drawSpec
            )

            h, w, c = frame.shape
            for idx, lm in enumerate(hand_landmarks.landmark):
                cx, cy = int(lm.x * w), int(lm.y * h)
                self.lmList.append([idx, cx, cy])

        return frame, img

    def frame_combine(self, frame, src):
        if len(frame.shape) == 3:
            frameH, frameW = frame.shape[:2]
            srcH, srcW = src.shape[:2]
            dst = np.zeros((max(frameH, srcH), frameW + srcW, 3), np.uint8)
            dst[:, :frameW] = frame[:, :]
            dst[:, frameW:] = src[:, :]
        else:
            src = cv.cvtColor(src, cv.COLOR_BGR2GRAY)
            frameH, frameW = frame.shape[:2]
            imgH, imgW = src.shape[:2]
            dst = np.zeros((frameH, frameW + imgW), np.uint8)
            dst[:, :frameW] = frame[:, :]
            dst[:, frameW:] = src[:, :]
        return dst

    def fingersUp(self):
        fingers = []

        # 拇指单独判断
        if (self.calc_angle(self.tipIds[0], self.tipIds[0] - 1, self.tipIds[0] - 2) > 150.0) and \
           (self.calc_angle(self.tipIds[0] - 1, self.tipIds[0] - 2, self.tipIds[0] - 3) > 150.0):
            fingers.append(1)
        else:
            fingers.append(0)

        # 另外四根看指尖和关节位置
        for idx in range(1, 5):
            if self.lmList[self.tipIds[idx]][2] < self.lmList[self.tipIds[idx] - 2][2]:
                fingers.append(1)
            else:
                fingers.append(0)

        return fingers

    def get_gesture(self):
        gesture = ""
        fingers = self.fingersUp()

        if self.lmList[self.tipIds[0]][2] > self.lmList[self.tipIds[1]][2] and \
           self.lmList[self.tipIds[0]][2] > self.lmList[self.tipIds[2]][2] and \
           self.lmList[self.tipIds[0]][2] > self.lmList[self.tipIds[3]][2] and \
           self.lmList[self.tipIds[0]][2] > self.lmList[self.tipIds[4]][2]:
            gesture = "Thumb_down"

        elif self.lmList[self.tipIds[0]][2] < self.lmList[self.tipIds[1]][2] and \
             self.lmList[self.tipIds[0]][2] < self.lmList[self.tipIds[2]][2] and \
             self.lmList[self.tipIds[0]][2] < self.lmList[self.tipIds[3]][2] and \
             self.lmList[self.tipIds[0]][2] < self.lmList[self.tipIds[4]][2] and \
             self.calc_angle(self.tipIds[1] - 1, self.tipIds[1] - 2, self.tipIds[1] - 3) < 150.0:
            gesture = "Thumb_up"

        if fingers.count(1) == 3 or fingers.count(1) == 4:
            if fingers[0] == 1 and (
                self.get_dist(self.lmList[4][1:], self.lmList[8][1:]) <
                self.get_dist(self.lmList[4][1:], self.lmList[5][1:])
            ):
                gesture = "OK"
            elif fingers[2] == fingers[3] == 0:
                gesture = "Rock"
            elif fingers.count(1) == 3:
                gesture = "Three"
            else:
                gesture = "Four"
        elif fingers.count(1) == 0:
            gesture = "Zero"
        elif fingers.count(1) == 1:
            gesture = "One"
        elif fingers.count(1) == 2:
            if fingers[0] == 1 and fingers[4] == 1:
                gesture = "Six"
            elif fingers[0] == 1 and self.calc_angle(4, 5, 8) > 90:
                gesture = "Eight"
            elif fingers[0] == 1 and fingers[1] == 1 and \
                    self.get_dist(self.lmList[4][1:], self.lmList[8][1:]) < 50:
                gesture = "Heart_single"
            else:
                gesture = "Two"
        elif fingers.count(1) == 5:
            gesture = "Five"

        if self.get_dist(self.lmList[4][1:], self.lmList[8][1:]) < 60 and \
           self.get_dist(self.lmList[4][1:], self.lmList[12][1:]) < 60 and \
           self.get_dist(self.lmList[4][1:], self.lmList[16][1:]) < 60 and \
           self.get_dist(self.lmList[4][1:], self.lmList[20][1:]) < 60:
            gesture = "Seven"

        if self.lmList[self.tipIds[0]][2] < self.lmList[self.tipIds[1]][2] and \
           self.lmList[self.tipIds[0]][2] < self.lmList[self.tipIds[2]][2] and \
           self.lmList[self.tipIds[0]][2] < self.lmList[self.tipIds[3]][2] and \
           self.lmList[self.tipIds[0]][2] < self.lmList[self.tipIds[4]][2] and \
           self.calc_angle(self.tipIds[1] - 1, self.tipIds[1] - 2, self.tipIds[1] - 3) > 150.0:
            gesture = "Eight"

        return gesture


def open_serial(port='/dev/serial0', baudrate=115200):
    if serial is None:
        print("没有 serial 模块，串口部分先跳过")
        return None

    try:
        ser = serial.Serial(port, baudrate, timeout=0.1)
        print("串口打开成功:", port, baudrate)
        return ser
    except Exception as e:
        print("串口打开失败:", e)
        return None


def send_gesture(ser, gesture):
    if ser is None or not gesture:
        return

    try:
        # 先直接发手势字符串
        ser.write((gesture + '\n').encode('utf-8'))
    except Exception as e:
        print("串口发送失败:", e)


'''
Zero One Two Three Four Five Six Seven Eight
OK: OK
Rock: rock
Thumb_up: 点赞
Thumb_down: 拇指向下
Heart_single: 单手比心
'''


if __name__ == '__main__':
    cv.startWindowThread()

    picam2 = Picamera2()
    config = picam2.create_preview_configuration(
        main={"format": 'RGB888', "size": (640, 480)}
    )
    config["transform"] = libcamera.Transform(hflip=0, vflip=1)
    picam2.configure(config)
    picam2.start()

    ser = open_serial('/dev/serial0', 115200)

    pTime = 0
    cTime = 0
    hand_detector = handDetector(maxHands=1, detectorCon=0.75)

    last_gesture = ""
    last_send_time = 0
    send_interval = 0.3

    while True:
        frame = picam2.capture_array()
        # frame = cv.flip(frame, 1)

        frame, img = hand_detector.findHands(frame, draw=False)

        if len(hand_detector.lmList) != 0:
            totalFingers = hand_detector.get_gesture()
            cv.rectangle(frame, (0, 430), (260, 480), (0, 255, 0), cv.FILLED)
            cv.putText(frame, str(totalFingers), (10, 470),
                       cv.FONT_HERSHEY_PLAIN, 2, (255, 0, 0), 2)

            now = time.time()
            if totalFingers != "" and (
                totalFingers != last_gesture or (now - last_send_time) > send_interval
            ):
                send_gesture(ser, totalFingers)
                last_gesture = totalFingers
                last_send_time = now

        if cv.waitKey(1) & 0xFF == ord('q'):
            break

        cTime = time.time()
        if cTime != pTime:
            fps = 1 / (cTime - pTime)
        else:
            fps = 0
        pTime = cTime

        text = "FPS : " + str(int(fps))
        cv.putText(frame, text, (10, 30),
                   cv.FONT_HERSHEY_SIMPLEX, 0.9, (0, 0, 255), 1)

        dist = hand_detector.frame_combine(frame, img)
        cv.imshow('dist', dist)

    if ser is not None:
        ser.close()

    picam2.stop()
    cv.destroyAllWindows()
