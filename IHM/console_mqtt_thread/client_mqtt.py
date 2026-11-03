"""Communication MQTT avec le capteur TempHumiX (paho-mqtt 2.x)."""
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

        self.client = mqtt_client.Client(
            callback_api_version=mqtt_client.CallbackAPIVersion.VERSION2,
            client_id=client_id,
            protocol=mqtt_client.MQTTv311,
        )
        if self.username:
            self.client.username_pw_set(self.username, self.password)
        self.client.on_connect = self._on_connect
        self.client.on_message = self._on_message

    def _on_connect(self, client, userdata, flags, reason_code, properties):
        if reason_code.is_failure:
            print(f"Échec de connexion MQTT : {reason_code}")
            return
        print(f"Connecté au broker MQTT : {self.ip_address}:{self.port}")
        # Les abonnements sont restaurés après une reconnexion.
        for topic, qos in self.subscriptions:
            client.subscribe(topic, qos=qos)

    def _on_message(self, client, userdata, msg):
        valeur = msg.payload.decode("utf-8", errors="replace")
        print(f"Réception : {valeur} depuis '{msg.topic}'")
        nom = msg.topic.rsplit("/", 1)[-1].upper()
        if nom.startswith("VERSION_"):
            self.version = valeur
        elif nom.startswith("TEMPERATURE_"):
            self.temperature = valeur
        elif nom.startswith("HUMIDITY_"):
            self.humidite = valeur
        elif nom.startswith("GET_DATE_"):
            self.date_IdO = valeur

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
        # Avant CONNACK, l'abonnement sera effectué dans _on_connect.
        if self.client.is_connected():
            self.client.subscribe(topic, qos=qos)
        print(f"Attente du topic : '{topic}'")

    def wait_mqtt(self, timeout=0):
        if timeout == 0:
            self.client.loop_forever()
        else:
            self.client.loop(timeout=timeout)
