"""Client MQTT en lecture : section 3.3 du TP."""
from client_mqtt import ClientMQTT
from config import BROKER, PORT, ID_CAPTEUR, USERNAME, PASSWORD, TOPICS_LECTURE


def main():
    print("Démarrage du client MQTT en lecture")
    mqtt = ClientMQTT(BROKER, PORT, f"Client_{ID_CAPTEUR}", USERNAME, PASSWORD)
    try:
        mqtt.connect_mqtt()
        for topic in TOPICS_LECTURE:
            mqtt.subscribe_mqtt(topic)
        print("Attente des données (Ctrl+C pour arrêter)...")
        mqtt.wait_mqtt(timeout=0)
    except (OSError, ConnectionError) as exc:
        print(f"Erreur de connexion : {exc}")
    except KeyboardInterrupt:
        print("Arrêt demandé")
    finally:
        mqtt.disconnect_mqtt()
        print("Fin de l'application")


if __name__ == "__main__":
    main()
