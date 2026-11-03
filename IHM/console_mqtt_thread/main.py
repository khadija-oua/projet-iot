"""Client console unifié : lecture dans ThreadMQTT, écriture dans le menu."""
from client_mqtt import ClientMQTT
from config import BROKER, PORT, ID_CAPTEUR, USERNAME, PASSWORD, TOPICS_LECTURE
from thread_mqtt import ThreadMQTT
from commandes import menu


def main():
    print("Démarrage du client MQTT unifié")
    mqtt = ClientMQTT(BROKER, PORT, f"Client_T{ID_CAPTEUR}", USERNAME, PASSWORD)
    thread = None
    try:
        mqtt.connect_mqtt()
        for topic in TOPICS_LECTURE:
            mqtt.subscribe_mqtt(topic)
        thread = ThreadMQTT(mqtt)
        thread.start()
        menu(mqtt)
    except (OSError, ConnectionError, RuntimeError, TimeoutError) as exc:
        print(f"Erreur MQTT : {exc}")
    except (KeyboardInterrupt, EOFError):
        print("Arrêt demandé")
    finally:
        if thread:
            thread.stopthread()
        mqtt.disconnect_mqtt()
        if thread:
            thread.join(timeout=3)
        print("Fin de l'application")


if __name__ == "__main__":
    main()
