"""Client MQTT en écriture : section 3.4 du TP."""
from client_mqtt import ClientMQTT
from config import BROKER, PORT, ID_CAPTEUR, USERNAME, PASSWORD
from commandes import menu


def main():
    print("Démarrage du client MQTT en écriture")
    # ID distinct du client de lecture pour les exécuter en parallèle.
    mqtt = ClientMQTT(BROKER, PORT, f"Client_E{ID_CAPTEUR}", USERNAME, PASSWORD)
    started = False
    try:
        mqtt.connect_mqtt()
        mqtt.client.loop_start()
        started = True
        menu(mqtt)
    except (OSError, ConnectionError, RuntimeError, TimeoutError) as exc:
        print(f"Erreur MQTT : {exc}")
    except (KeyboardInterrupt, EOFError):
        print("Arrêt demandé")
    finally:
        mqtt.disconnect_mqtt()
        if started:
            mqtt.client.loop_stop()
        print("Fin de l'application")


if __name__ == "__main__":
    main()
