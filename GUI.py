import sys
import serial
import threading
import pyqtgraph as pg 
from PyQt5 import uic
from PyQt5.QtWidgets import (
    QApplication, QWidget, QLabel, QTextEdit,
    QVBoxLayout, QMainWindow
)
from PyQt5.QtCore import pyqtSignal, QObject


class SerialHandler(QObject):
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
        #self.ser.close()

    def send(self, val):
        message = f"{int(val):03d}"
        self.ser.write(message.encode('ascii'))
        


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        uic.loadUi("main.ui", self)

        self.setWindowTitle("DC MOTOR GUI - Speed control tracking")

        # data storage
        self.max_pts = 1000
        self.data1 = []
        self.data2 = []
        self.data3 = []

        self.plot = pg.PlotWidget()
        #self.plot.addLegend()
        self.plot.showGrid(x=False, y=True)
        self.plot.getAxis('bottom').setPen(pg.mkPen('k'))
        self.plot.getAxis('bottom').setTextPen(pg.mkPen('k'))
        self.plot.setBackground('k')

        self.curve1 = self.plot.plot(name="Y_ref", pen=pg.mkPen(color=(255, 0, 0, 200), width=1.5))
        self.curve2 = self.plot.plot(name="Y", pen=pg.mkPen(color=(50, 205, 50, 255), width=1.5))

        self.viewbox_U = pg.ViewBox()
        self.plot.scene().addItem(self.viewbox_U)
        self.viewbox_U.setXLink(self.plot)

        self.viewbox_U.setYRange(0, 1000, padding=0)
        self.plot.setYRange(0, 310, padding=0)

        self.curve3 = pg.PlotDataItem(pen=pg.mkPen(color=(255, 0, 255, 100), width=1.5))
        self.viewbox_U.addItem(self.curve3)

        axisU = pg.AxisItem('right')
        axisU.setPen(pg.mkPen('w'))
        self.plot.plotItem.layout.addItem(axisU, 2, 2)
        axisU.linkToView(self.viewbox_U)
        
        self.legend = self.plot.addLegend()

        self.curve1 = self.plot.plot(name="Y_ref", pen=pg.mkPen(color=(255, 0, 0, 200), width=1.5))
        self.curve2 = self.plot.plot(name="Y", pen=pg.mkPen(color=(50, 205, 50, 255), width=1.5))

        self.curve3 = pg.PlotDataItem(pen=pg.mkPen(color=(255, 0, 255, 100), width=1.5))
        self.viewbox_U.addItem(self.curve3)

        self.legend.addItem(self.curve3, "U")


        def update_views():
            self.viewbox_U.setGeometry(self.plot.getPlotItem().vb.sceneBoundingRect())
            self.viewbox_U.linkedViewChanged(self.plot.getPlotItem().vb, self.viewbox_U.XAxis)
        update_views()

        layout = QVBoxLayout(self.plot_widget)
        layout.addWidget(self.plot)

        # serial port handling
        self.serial_port_handler = SerialHandler(port="COM3", baudrate=57600)
        self.serial_port_handler.data_received.connect(self.update_ui)

        self.thread = threading.Thread(target=self.serial_port_handler.start, daemon=True)
        self.thread.start()
    
        self.horizontalSlider.valueChanged.connect(lambda val: self.serial_port_handler.send(val))

    def set_running(self, bool):
        self.serial_port_handler.running = bool

    def update_ui(self, v1, v2, v3):
        # lcd shows
        self.lcdNumber_1.display(v1)
        self.lcdNumber_2.display(v2)
        self.lcdNumber_3.display(v3)

        # progress bar
        self.progressBar.setValue(int(v1))
        self.progressBar_2.setValue(int(v2))
        self.progressBar_3.setValue(int(v3))

        # slider
        self.horizontalSlider.blockSignals(True)
        self.horizontalSlider.setValue(int(v1))
        self.horizontalSlider.blockSignals(False)

        # plot handling
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
        self.serial_port_handler.stop()
        event.accept()


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec_())
