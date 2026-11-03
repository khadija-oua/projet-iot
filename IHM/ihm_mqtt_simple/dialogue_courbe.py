"""Section 6 : consultation des mesures MQTT archivees dans SQLite."""
from datetime import datetime, timedelta
from pathlib import Path
import sqlite3

from PySide6.QtCore import QDateTime, Qt
from PySide6.QtGui import QImage, QPixmap
from PySide6.QtWidgets import QDialog, QGraphicsPixmapItem, QGraphicsScene, QMessageBox
from matplotlib.figure import Figure
from matplotlib.backends.backend_agg import FigureCanvasAgg

from graphe import Ui_Dialog

BASE_SQLITE = Path(__file__).resolve().parent / "IOT.db"
FORMAT_DATE = "%d/%m/%Y %H:%M:%S"


class CourbeDialog(QDialog, Ui_Dialog):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setupUi(self)
        self.setWindowTitle("TempHumiX - Historique des mesures")
        self.scene = QGraphicsScene(self)
        self.graphicsView.setScene(self.scene)

        # Par defaut, choisir la derniere heure contenant des mesures.
        fin = self._derniere_date() or datetime.now()
        debut = fin - timedelta(hours=1)
        self.dteStartTime.setDateTime(QDateTime.fromString(debut.strftime(FORMAT_DATE), "dd/MM/yyyy HH:mm:ss"))
        self.dteEndTime.setDateTime(QDateTime.fromString(fin.strftime(FORMAT_DATE), "dd/MM/yyyy HH:mm:ss"))

        self.btnLoadData.clicked.connect(self.afficher_courbe)
        self.btnClearData.clicked.connect(self.effacer_courbe)

    def _lire_enregistrements(self):
        if not BASE_SQLITE.is_file():
            raise FileNotFoundError(f"Base SQLite introuvable : {BASE_SQLITE}")
        with sqlite3.connect(BASE_SQLITE) as connexion:
            return connexion.execute(
                'SELECT Date, Temperature, Humidite FROM IOT_DATA'
            ).fetchall()

    def _derniere_date(self):
        try:
            dates = []
            for date_texte, _, _ in self._lire_enregistrements():
                try:
                    dates.append(datetime.strptime(date_texte, FORMAT_DATE))
                except (ValueError, TypeError):
                    continue
            return max(dates) if dates else None
        except (sqlite3.Error, FileNotFoundError):
            return None

    def afficher_courbe(self):
        debut = self.dteStartTime.dateTime().toPython()
        fin = self.dteEndTime.dateTime().toPython()
        if debut > fin:
            QMessageBox.warning(self, "Dates invalides", "La date de debut doit preceder la date de fin.")
            return
        try:
            lignes = self._lire_enregistrements()
        except (sqlite3.Error, OSError) as erreur:
            QMessageBox.warning(self, "Lecture SQLite", str(erreur))
            return

        mesures = []
        for date_texte, temperature, humidite in lignes:
            try:
                instant = datetime.strptime(date_texte, FORMAT_DATE)
                if debut <= instant <= fin and temperature is not None and humidite is not None:
                    mesures.append((instant, float(temperature), float(humidite)))
            except (ValueError, TypeError):
                continue
        mesures.sort(key=lambda mesure: mesure[0])
        if not mesures:
            self.scene.clear()
            QMessageBox.information(self, "Historique", "Aucune mesure pour cet intervalle.")
            return

        heures = [mesure[0] for mesure in mesures]
        temperatures = [mesure[1] for mesure in mesures]
        humidites = [mesure[2] for mesure in mesures]

        figure = Figure(figsize=(7.5, 4.5), dpi=120, layout="constrained")
        axe = figure.add_subplot(111)
        axe.plot(heures, temperatures, label="Temperature (°C)", marker="o", markersize=2)
        axe.plot(heures, humidites, label="Humidite (%)", marker="+", markersize=3)
        axe.set_title("Historique TempHumiX")
        axe.set_xlabel("Date et heure")
        axe.set_ylabel("Valeur")
        axe.grid(True)
        axe.legend()
        figure.autofmt_xdate(rotation=25)

        canvas = FigureCanvasAgg(figure)
        canvas.draw()
        largeur, hauteur = canvas.get_width_height()
        image = QImage(canvas.buffer_rgba(), largeur, hauteur, largeur * 4, QImage.Format.Format_RGBA8888).copy()
        pixmap = QPixmap.fromImage(image)
        self.scene.clear()
        self.scene.addItem(QGraphicsPixmapItem(pixmap))
        self.scene.setSceneRect(self.scene.itemsBoundingRect())
        self.graphicsView.fitInView(self.scene.sceneRect(), Qt.AspectRatioMode.KeepAspectRatio)

    def effacer_courbe(self):
        self.scene.clear()
