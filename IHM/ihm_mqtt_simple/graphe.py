# -*- coding: utf-8 -*-

################################################################################
## Form generated from reading UI file 'graphe.ui'
##
## Created by: Qt User Interface Compiler version 6.12.0
##
## WARNING! All changes made in this file will be lost when recompiling UI file!
################################################################################

from PySide6.QtCore import (QCoreApplication, QDate, QDateTime, QLocale,
    QMetaObject, QObject, QPoint, QRect,
    QSize, QTime, QUrl, Qt)
from PySide6.QtGui import (QBrush, QColor, QConicalGradient, QCursor,
    QFont, QFontDatabase, QGradient, QIcon,
    QImage, QKeySequence, QLinearGradient, QPainter,
    QPalette, QPixmap, QRadialGradient, QTransform)
from PySide6.QtWidgets import (QAbstractButton, QApplication, QDateTimeEdit, QDialog,
    QDialogButtonBox, QGraphicsView, QLabel, QPushButton,
    QSizePolicy, QWidget)

class Ui_Dialog(object):
    def setupUi(self, Dialog):
        if not Dialog.objectName():
            Dialog.setObjectName(u"Dialog")
        Dialog.resize(633, 557)
        self.buttonBox = QDialogButtonBox(Dialog)
        self.buttonBox.setObjectName(u"buttonBox")
        self.buttonBox.setGeometry(QRect(340, 470, 181, 32))
        self.buttonBox.setOrientation(Qt.Orientation.Horizontal)
        self.buttonBox.setStandardButtons(QDialogButtonBox.StandardButton.Cancel|QDialogButtonBox.StandardButton.Ok)
        self.dteStartTime = QDateTimeEdit(Dialog)
        self.dteStartTime.setObjectName(u"dteStartTime")
        self.dteStartTime.setGeometry(QRect(270, 30, 194, 26))
        self.dteEndTime = QDateTimeEdit(Dialog)
        self.dteEndTime.setObjectName(u"dteEndTime")
        self.dteEndTime.setGeometry(QRect(270, 70, 194, 26))
        self.label = QLabel(Dialog)
        self.label.setObjectName(u"label")
        self.label.setGeometry(QRect(110, 30, 141, 16))
        self.label_2 = QLabel(Dialog)
        self.label_2.setObjectName(u"label_2")
        self.label_2.setGeometry(QRect(110, 80, 131, 16))
        self.graphicsView = QGraphicsView(Dialog)
        self.graphicsView.setObjectName(u"graphicsView")
        self.graphicsView.setGeometry(QRect(100, 140, 361, 291))
        self.btnLoadData = QPushButton(Dialog)
        self.btnLoadData.setObjectName(u"btnLoadData")
        self.btnLoadData.setGeometry(QRect(50, 470, 131, 26))
        self.btnClearData = QPushButton(Dialog)
        self.btnClearData.setObjectName(u"btnClearData")
        self.btnClearData.setGeometry(QRect(200, 470, 121, 26))

        self.retranslateUi(Dialog)
        self.buttonBox.accepted.connect(Dialog.accept)
        self.buttonBox.rejected.connect(Dialog.reject)

        QMetaObject.connectSlotsByName(Dialog)
    # setupUi

    def retranslateUi(self, Dialog):
        Dialog.setWindowTitle(QCoreApplication.translate("Dialog", u"Dialog", None))
        self.dteStartTime.setDisplayFormat(QCoreApplication.translate("Dialog", u"dd/MM/yyyy HH:mm:ss", None))
        self.dteEndTime.setDisplayFormat(QCoreApplication.translate("Dialog", u"dd/MM/yyyy HH:mm:ss", None))
        self.label.setText(QCoreApplication.translate("Dialog", u"Date et heure de d\u00e9but :", None))
        self.label_2.setText(QCoreApplication.translate("Dialog", u"Date et heure de fin :", None))
        self.btnLoadData.setText(QCoreApplication.translate("Dialog", u"Charger les donn\u00e9es", None))
        self.btnClearData.setText(QCoreApplication.translate("Dialog", u"Effacer les courbes", None))
    # retranslateUi

