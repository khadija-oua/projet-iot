"""Client MQTT TempHumiX avec archivage SQLite (section 5)."""
import sqlite3
from pathlib import Path
from paho.mqtt import client as mqtt_client


class ClientMQTT:
    def __init__(self, ip_address, port, client_id, username="", password=""):
        self.ip_address = ip_address
        self.port = port
        self.client_id = client_id
        self.username = username
        self.password = password
        self.version = None
        self.temperature = None
        self.humidite = None
        self.date_IdO = None
        self.subscriptions = []
        self.conn = None
        self.tablename = "IOT_DATA"
        self._derniere_date_archivee = None

        self.client = mqtt_client.Client(
            callback_api_version=mqtt_client.CallbackAPIVersion.VERSION2,
            client_id=client_id,
            protocol=mqtt_client.MQTTv311,
        )
        if username:
            self.client.username_pw_set(username, password)
        self.client.on_connect = self._on_connect
        self.client.on_message = self._on_message

    def connect_sqlite(self, nom_base=None, nom_table="IOT_DATA"):
        """Créer la base et la table depuis le thread de réception."""
        if nom_base is None:
            nom_base = Path(__file__).resolve().parent / "IOT.db"
        if nom_table != "IOT_DATA":
            raise ValueError("Nom de table non autorisé")
        self.tablename = nom_table
        self.conn = sqlite3.connect(str(nom_base))
        self.conn.execute(
            "CREATE TABLE IF NOT EXISTS IOT_DATA ("
            "Id_capteur INTEGER, Date TEXT, Version TEXT, "
            "Temperature REAL, Humidite REAL)"
        )
        self.conn.commit()
        print(f"Base SQLite ouverte : {nom_base}")

    def disconnect_sqlite(self):
        if self.conn is not None:
            self.conn.close()
            self.conn = None
            print("Base SQLite fermée")

    def _on_connect(self, client, userdata, flags, reason_code, properties):
        if reason_code.is_failure:
            print(f"Échec de connexion MQTT : {reason_code}")
            return
        print(f"Connecté au broker MQTT : {self.ip_address}:{self.port}")
        for topic, qos in self.subscriptions:
            client.subscribe(topic, qos=qos)

    def _on_message(self, client, userdata, msg):
        valeur = msg.payload.decode("utf-8", errors="replace")
        print(f"Réception : {valeur} depuis '{msg.topic}'")
        nom = msg.topic.rsplit("/", 1)[-1].upper()

        # Vérifier que le message provient du capteur configuré.
        try:
            from config import ID_CAPTEUR
            id_attendu = str(ID_CAPTEUR)
        except ImportError:
            id_attendu = "48763"
        if not nom.endswith("_" + id_attendu):
            return

        if nom.startswith("VERSION_"):
            self.version = valeur
        elif nom.startswith("TEMPERATURE_"):
            try:
                self.temperature = float(valeur)
            except ValueError:
                print("Température invalide ignorée")
        elif nom.startswith("HUMIDITY_"):
            try:
                self.humidite = float(valeur)
            except ValueError:
                print("Humidité invalide ignorée")
        elif nom.startswith("GET_DATE_"):
            self.date_IdO = valeur
            # Arduino publie GET_DATE après TEMPERATURE et HUMIDITY.
            if (self.conn is not None
                    and self.temperature is not None
                    and self.humidite is not None
                    and valeur != self._derniere_date_archivee):
                try:
                    self.conn.execute(
                        "INSERT INTO IOT_DATA "
                        "(Id_capteur, Date, Version, Temperature, Humidite) "
                        "VALUES (?, ?, ?, ?, ?)",
                        (int(id_attendu), self.date_IdO, self.version or "",
                         self.temperature, self.humidite),
                    )
                    self.conn.commit()
                    self._derniere_date_archivee = valeur
                    print(f"Mesure archivée : {valeur} | "
                          f"{self.temperature} °C | {self.humidite} %")
                except sqlite3.Error as erreur:
                    print(f"Erreur SQLite : {erreur}")

    def connect_mqtt(self) -> mqtt_client.Client:
        self.client.connect(self.ip_address, self.port, keepalive=60)
        return self.client

    def disconnect_mqtt(self) -> None:
        self.client.disconnect()

    def publish_mqtt(self, topic, message, qos=0):
        info = self.client.publish(topic, payload=str(message), qos=qos)
        if info.rc != mqtt_client.MQTT_ERR_SUCCESS:
            raise RuntimeError(f"Publication impossible (code {info.rc})")
        info.wait_for_publish(timeout=5)
        if not info.is_published():
            raise TimeoutError(f"Publication non terminée : {topic}")
        print(f"Publication : {message} sur '{topic}'")

    def subscribe_mqtt(self, topic, qos=0):
        if (topic, qos) not in self.subscriptions:
            self.subscriptions.append((topic, qos))
        if self.client.is_connected():
            self.client.subscribe(topic, qos=qos)
        print(f"Attente du topic : '{topic}'")

    def wait_mqtt(self, timeout=0):
        if timeout == 0:
            self.client.loop_forever()
        else:
            self.client.loop(timeout=timeout)
