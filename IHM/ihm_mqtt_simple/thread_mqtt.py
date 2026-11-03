"""Thread MQTT avec ouverture et fermeture de SQLite (section 5)."""
from threading import Thread, Event


class ThreadMQTT(Thread):
    def __init__(self, client_mqtt):
        super().__init__(daemon=True)
        self.mqtt = client_mqtt
        self._stop_event = Event()

    def run(self):
        # SQLite est utilisée uniquement depuis ce thread.
        try:
            self.mqtt.connect_sqlite()
            while not self._stop_event.is_set():
                self.mqtt.wait_mqtt(timeout=1.0)
        finally:
            self.mqtt.disconnect_sqlite()

    def stopthread(self):
        self._stop_event.set()
