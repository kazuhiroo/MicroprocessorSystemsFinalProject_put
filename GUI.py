import sys
import serial
import threading
import pyqtgraph as pg 
from PyQt5.QtWidgets import (
    QApplication, QWidget, QLabel, QTextEdit,
    QVBoxLayout, QHBoxLayout
)
from PyQt5.QtCore import pyqtSignal, QObject


class SerialReader(QObject):
    data_received = pyqtSignal(float, float, float)

    def __init__(self, port, baudrate=9600):
        super().__init__()
        self.ser = serial.Serial(port, baudrate, timeout=1)
        self.running = True

    def start(self):
        while self.running:
            try:
                line = self.ser.readline().decode().strip()
                if line:
                    parts = line.split()
                    if len(parts) == 3:
                        v1, v2, v3 = map(float, parts)
                        self.data_received.emit(v1, v2, v3)
            except Exception as e:
                print("Błąd:", e)

    def stop(self):
        self.running = False
        self.ser.close()


class MainWindow(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("DC MOTOR GUI - Speed control tracking")

        # data history text edit (just in case)
        self.text_edit = QTextEdit()
        self.text_edit.setReadOnly(True)

        # labels
        self.label1 = QLabel("Y_ref: ---")
        self.label2 = QLabel("Y: ---")
        self.label3 = QLabel("U: ---")

        labels_layout = QVBoxLayout()
        labels_layout.addWidget(self.label1)
        labels_layout.addWidget(self.label2)
        labels_layout.addWidget(self.label3)
        labels_layout.addStretch()

        # data storage\
        self.max_pts = 10
        self.data1 = []
        self.data2 = []
        self.data3 = []

        # plot
        self.plot = pg.PlotWidget(title = "UART received data")
        self.plot.addLegend()
        self.plot.showGrid(x=False, y=True)
        self.plot.getAxis('bottom').setPen(pg.mkPen('k'))
        self.plot.getAxis('bottom').setTextPen(pg.mkPen('k'))
        self.plot.setBackground('k')

        # curves
        self.curve1 = self.plot.plot(name="Y_ref", pen=pg.mkPen(color = (255, 0, 0, 100), width=1.5))
        self.curve2 = self.plot.plot(name="Y", pen=pg.mkPen(color = (50, 205, 50, 255), width=1.5))
        self.curve3 = self.plot.plot(name="U", pen=pg.mkPen(color = (255, 0, 255, 255), width=1.5))

        main_layout = QHBoxLayout()
        main_layout.addWidget(self.plot)
        main_layout.addLayout(labels_layout)

        self.setLayout(main_layout)

        # serial port handling
        self.reader = SerialReader(port="COM3", baudrate=57600)
        self.reader.data_received.connect(self.update_ui)

        self.thread = threading.Thread(target=self.reader.start, daemon=True)
        self.thread.start()

    def update_ui(self, v1, v2, v3):
        #self.text_edit.append(f"{v1:.3f}  {v2:.3f}  {v3:.3f}")
        self.label1.setText(f"Y_ref: {v1:.3f}")
        self.label2.setText(f"Y: {v2:.3f}")
        self.label3.setText(f"U: {v3:.3f}")

        if len(self.data1) >= self.max_pts:
            self.data1.pop(0)
            self.data2.pop(0)
            self.data3.pop(0)
        
        self.data1.append(v1)
        self.data2.append(v2)
        self.data3.append(v3)

        self.curve1.setData(self.data1)
        self.curve2.setData(self.data2)
        self.curve3.setData(self.data3)


    def closeEvent(self, event):
        self.reader.stop()
        event.accept()


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainWindow()
    window.resize(600, 300)
    window.show()
    sys.exit(app.exec_())
