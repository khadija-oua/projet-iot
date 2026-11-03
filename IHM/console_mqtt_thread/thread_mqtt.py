"""Boucle de réception MQTT dans un thread séparé : section 3.6."""
from threading import Thread, Event


class ThreadMQTT(Thread):
    def __init__(self, client_mqtt):
        super().__init__(daemon=True)
        self.mqtt = client_mqtt
        self._stop_event = Event()

    def run(self):
        while not self._stop_event.is_set():
            self.mqtt.wait_mqtt(timeout=1.0)

    def stopthread(self):
        self._stop_event.set()
