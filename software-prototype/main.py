"""Webcam OCR prototype: find text in the camera feed, overlay it.

Stand-in for AR glasses display until real hardware/SDK is in the loop —
a webcam window plays the role of the glass lens.
"""
import sys

import cv2
import pytesseract


def filter_boxes(boxes: list[dict], min_confidence: int = 60) -> list[dict]:
    """Drop low-confidence/empty OCR results."""
    return [b for b in boxes if b["text"].strip() and b["conf"] >= min_confidence]


def detect_text(frame) -> list[dict]:
    data = pytesseract.image_to_data(frame, output_type=pytesseract.Output.DICT)
    boxes = [
        {
            "text": data["text"][i],
            "conf": int(data["conf"][i]),
            "box": (data["left"][i], data["top"][i], data["width"][i], data["height"][i]),
        }
        for i in range(len(data["text"]))
    ]
    return filter_boxes(boxes)


def draw_overlay(frame, boxes: list[dict]):
    for b in boxes:
        x, y, w, h = b["box"]
        cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
        cv2.putText(frame, b["text"], (x, y - 5), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 1)
    return frame


def main():
    cap = cv2.VideoCapture(0)
    if not cap.isOpened():
        sys.exit("No camera found.")

    print("Press q to quit.")
    while True:
        ok, frame = cap.read()
        if not ok:
            break
        boxes = detect_text(frame)
        draw_overlay(frame, boxes)
        cv2.imshow("glance", frame)
        if cv2.waitKey(1) & 0xFF == ord("q"):
            break

    cap.release()
    cv2.destroyAllWindows()


def demo():
    sample = [
        {"text": "hello", "conf": 80, "box": (0, 0, 1, 1)},
        {"text": "  ", "conf": 90, "box": (0, 0, 1, 1)},
        {"text": "noise", "conf": 10, "box": (0, 0, 1, 1)},
    ]
    assert filter_boxes(sample) == [sample[0]]


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "test":
        demo()
        print("ok")
    else:
        main()
