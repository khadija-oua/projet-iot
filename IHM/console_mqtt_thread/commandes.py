"""Commandes du menu console selon la section 3.4 du TP."""
from config import TOPIC_COMMAND, TOPIC_BARGRAPH, TOPIC_BLINK


def envoyer_choix(mqtt, choix):
    if choix == "1":
        mqtt.publish_mqtt(TOPIC_COMMAND, 10)
    elif choix == "2":
        mqtt.publish_mqtt(TOPIC_COMMAND, 20)
        mqtt.publish_mqtt(TOPIC_COMMAND, 50)  # Mesures en marche : clignotement rapide.
    elif choix == "3":
        mqtt.publish_mqtt(TOPIC_COMMAND, 30)
        mqtt.publish_mqtt(TOPIC_COMMAND, 40)  # Mesures arrêtées : clignotement lent.
    elif choix == "4":
        mqtt.publish_mqtt(TOPIC_COMMAND, 40)
    elif choix == "5":
        mqtt.publish_mqtt(TOPIC_COMMAND, 50)
    elif choix == "6":
        try:
            valeur = int(input("Période en ms (multiple de 100) : "))
        except ValueError:
            print("Entrez un nombre entier.")
            return
        if valeur <= 0 or valeur % 100 != 0:
            print("La période doit être positive et multiple de 100 ms.")
            return
        mqtt.publish_mqtt(TOPIC_BLINK, valeur)
        mqtt.publish_mqtt(TOPIC_COMMAND, 60)
    elif choix == "7":
        try:
            valeur = int(input("Valeur du bargraphe (0 à 1023) : "))
        except ValueError:
            print("Entrez un nombre entier.")
            return
        if not 0 <= valeur <= 1023:
            print("La valeur doit être comprise entre 0 et 1023.")
            return
        mqtt.publish_mqtt(TOPIC_BARGRAPH, valeur)
    else:
        print("Choix inconnu.")


def menu(mqtt):
    while True:
        print("\nConsole capteur IdO")
        print("1 - Mise à l'heure du capteur")
        print("2 - Démarrage des mesures")
        print("3 - Arrêt des mesures")
        print("4 - Clignotement lent")
        print("5 - Clignotement rapide")
        print("6 - Clignotement personnalisé")
        print("7 - Écriture du bargraphe")
        print("Q - Quitter")
        choix = input("Votre choix : ").strip().upper()
        if choix == "Q":
            print("Arrêt du client MQTT")
            break
        envoyer_choix(mqtt, choix)
