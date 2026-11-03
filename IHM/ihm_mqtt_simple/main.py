"""Section 4.5 : interface PySide6 de TempHumiX."""
import os
import sys

from PySide6.QtCore import QTimer
from PySide6.QtGui import QIntValidator
from PySide6.QtWidgets import QApplication, QMainWindow, QMessageBox

from mon_ihm import Ui_MainWindow
from client_mqtt import ClientMQTT
from thread_mqtt import ThreadMQTT
from config import ID_CAPTEUR, TOPICS_LECTURE, TOPIC_COMMAND, TOPIC_BLINK, TOPIC_BARGRAPH

from PySide6.QtWidgets import QPushButton
from dialogue_courbe import CourbeDialog

class MainWindow(QMainWindow, Ui_MainWindow):
    def __init__(self):
        super().__init__()
        self.setupUi(self)
        # Bouton d'ouverture de l'historique
        self.btnGraph = QPushButton("Afficher les courbes", self.centralwidget)
        self.btnGraph.setGeometry(450, 400, 200, 36)
        self.btnGraph.clicked.connect(self.ouvrirCourbes)
        self.setWindowTitle("TempHumiX - IHM MQTT")
        self.mqtt = None
        self.thread_mqtt = None

        # Compatibilité avec les noms présents dans mon_ihm.py.
        self.lblTemperature_2.setObjectName("lblValeurTemperature")
        self.lblDateTime.setMinimumWidth(190)
        self.lblTemperature_2.setMinimumWidth(85)
        self.lblHumidite.setMinimumWidth(75)

        self.lneUsername.setText(os.environ.get("IOT_MQTT_USER", ""))
        self.lnePassword.setText(os.environ.get("IOT_MQTT_PASSWORD", ""))
        self.lneBlink.setValidator(QIntValidator(100, 60000, self))
        self.lneBargraph.setValidator(QIntValidator(0, 1023, self))

        self.timer = QTimer(self)
        self.timer.setInterval(1000)
        self.timer.timeout.connect(self.display)

        self.btnConnect.clicked.connect(self.connectMQTT)
        self.btnDisconnect.clicked.connect(self.disconnectMQTT)
        self.btnSynchroDateTime.clicked.connect(lambda: self.envoyer_commande(10))
        self.btnStartMeasure.clicked.connect(self.startMeasure)
        self.btnStopMeasure.clicked.connect(self.stopMeasure)
        self.btnSlowBlink.clicked.connect(lambda: self.envoyer_commande(40))
        self.btnFastBlink.clicked.connect(lambda: self.envoyer_commande(50))
        self.btnBlink.clicked.connect(self.customBlink)
        self.lneBargraph.returnPressed.connect(self.setBargraph)

        self.btnDisconnect.setEnabled(False)
        self._commandes_actives(False)
        self.statusbar.showMessage("Déconnecté")

    def _commandes_actives(self, actif):
        for bouton in (
            self.btnSynchroDateTime, self.btnStartMeasure,
            self.btnStopMeasure, self.btnSlowBlink,
            self.btnFastBlink, self.btnBlink,
        ):
            bouton.setEnabled(actif)
        self.lneBargraph.setEnabled(actif)

    def connectMQTT(self):
        if self.mqtt is not None:
            return
        adresse = self.lneBroker.text().strip()
        utilisateur = self.lneUsername.text().strip()
        mot_de_passe = self.lnePassword.text()
        try:
            port = int(self.lnePort.text().strip())
            if not adresse or not (1 <= port <= 65535):
                raise ValueError("Adresse ou port invalide")
            self.mqtt = ClientMQTT(
                adresse, port, f"Client_IHM_{ID_CAPTEUR}",
                utilisateur, mot_de_passe
            )
            # Les abonnements seront activés à la connexion.
            for topic in TOPICS_LECTURE:
                self.mqtt.subscribe_mqtt(topic)
            self.mqtt.connect_mqtt()
            self.thread_mqtt = ThreadMQTT(self.mqtt)
            self.thread_mqtt.start()
            self.timer.start()
            self.btnConnect.setEnabled(False)
            self.btnDisconnect.setEnabled(True)
            self.statusbar.showMessage("Connexion MQTT en cours...")
        except (ValueError, OSError, RuntimeError) as erreur:
            self.mqtt = None
            self.thread_mqtt = None
            QMessageBox.warning(self, "Connexion MQTT", str(erreur))

    def display(self):
        if self.mqtt is None:
            return
        if not self.mqtt.client.is_connected():
            self._commandes_actives(False)
            self.statusbar.showMessage("En attente de connexion MQTT...")
            return
        self._commandes_actives(True)
        self.statusbar.showMessage("Connecté au broker MQTT")
        if self.mqtt.temperature is not None:
            self.lblTemperature_2.setText(f"{self.mqtt.temperature} °C")
        if self.mqtt.humidite is not None:
            self.lblHumidite.setText(f"{self.mqtt.humidite} %")
        if self.mqtt.date_IdO is not None:
            self.lblDateTime.setText(self.mqtt.date_IdO)

    def envoyer(self, topic, valeur):
        if self.mqtt is None or not self.mqtt.client.is_connected():
            QMessageBox.warning(self, "MQTT", "Connecte-toi d'abord au broker.")
            return False
        try:
            self.mqtt.publish_mqtt(topic, valeur)
            self.statusbar.showMessage(f"Commande envoyée : {valeur}", 3000)
            return True
        except (OSError, RuntimeError, TimeoutError) as erreur:
            QMessageBox.warning(self, "Publication MQTT", str(erreur))
            return False

    def envoyer_commande(self, code):
        self.envoyer(TOPIC_COMMAND, code)

    def startMeasure(self):
        if self.envoyer(TOPIC_COMMAND, 20):
            self.envoyer(TOPIC_COMMAND, 50)

    def stopMeasure(self):
        if self.envoyer(TOPIC_COMMAND, 30):
            self.envoyer(TOPIC_COMMAND, 40)

    def customBlink(self):
        valeur = self.lneBlink.text().strip()
        if not valeur or int(valeur) % 100 != 0:
            QMessageBox.warning(self, "Période", "Saisir un multiple de 100 ms.")
            return
        if self.envoyer(TOPIC_BLINK, int(valeur)):
            self.envoyer(TOPIC_COMMAND, 60)

    def setBargraph(self):
        valeur = self.lneBargraph.text().strip()
        if valeur:
            self.envoyer(TOPIC_BARGRAPH, int(valeur))

    def disconnectMQTT(self):
        self.timer.stop()
        if self.thread_mqtt is not None:
            self.thread_mqtt.stopthread()
        if self.mqtt is not None:
            self.mqtt.disconnect_mqtt()
        if self.thread_mqtt is not None:
            self.thread_mqtt.join(timeout=2)
        self.thread_mqtt = None
        self.mqtt = None
        self.btnConnect.setEnabled(True)
        self.btnDisconnect.setEnabled(False)
        self._commandes_actives(False)
        self.statusbar.showMessage("Déconnecté")
    def ouvrirCourbes(self):
        dialogue = CourbeDialog(self)
        dialogue.exec()

    def closeEvent(self, event):
        self.disconnectMQTT()
        event.accept()


if __name__ == "__main__":
    app = QApplication(sys.argv)
    fenetre = MainWindow()
    fenetre.show()
    sys.exit(app.exec())
