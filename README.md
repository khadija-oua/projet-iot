# Projet IoT — TempHumiX

TempHumiX est un projet de mesure de température et d'humidité réalisé dans le cadre des TP IoT à **IMT Nord Europe**. Il a été développé progressivement : d'abord en simulation avec SimulIDE, puis sur une carte Arduino UNO R4 WiFi avec MQTT, et enfin avec une interface de supervision en Python.

Le boîtier mesure la température et l'humidité avec un capteur DHT22, affiche les informations sur un LCD et échange des messages avec un PC. L'application Python permet de consulter les mesures, d'envoyer des commandes, de les archiver dans SQLite et de tracer leur évolution.

## Organisation du dépôt

L'arborescence ci-dessous reprend les **dossiers et les noms de fichiers suivis par Git**. Pour garder le schéma lisible, le contenu répétitif des dossiers `build/arduino.avr.uno/` est détaillé juste après.

```text
projet-iot/
├── README.md
├── SIMULIDE/
│   ├── clignotement_led/
│   │   ├── clignotement_led.ino
│   │   ├── clignotement_led.sim1
│   │   └── build/arduino.avr.uno/
│   ├── bargraphe/
│   │   ├── bargraphe.ino
│   │   ├── bargraphe.sim1
│   │   └── build/arduino.avr.uno/
│   ├── capteur_dht22/
│   │   ├── capteur_dht22.ino
│   │   ├── capteur_dht22.sim1
│   │   └── build/arduino.avr.uno/
│   ├── lcd_i2c/
│   │   ├── lcd_i2c.ino
│   │   ├── lcd_i2c.sim1
│   │   └── build/arduino.avr.uno/
│   ├── lcd_i2c_dht22/
│   │   ├── lcd_i2c_dht22.ino
│   │   ├── lcd_i2c_dht22.sim1
│   │   └── build/arduino.avr.uno/
│   └── TempHumiX/
│       ├── TempHumiX.ino
│       ├── TempHumiX.sim1
│       └── build/arduino.avr.uno/
├── TEMPHUMIX/
│   ├── acquisition/
│   │   └── acquisition.ino
│   ├── rtc/
│   │   └── rtc.ino
│   ├── wifi/
│   │   ├── .gitignore
│   │   └── wifi.ino
│   ├── mqtt/
│   │   ├── .gitignore
│   │   └── mqtt.ino
│   ├── commandes/
│   │   ├── .gitignore
│   │   └── commandes.ino
│   └── optimisation/
│       ├── .gitignore
│       └── optimisation.ino
└── IHM/
    ├── .gitignore
    ├── requirements.txt
    ├── console_mqtt_lecture/
    │   ├── client_mqtt.py
    │   ├── config.py
    │   └── main.py
    ├── console_mqtt_ecriture/
    │   ├── client_mqtt.py
    │   ├── commandes.py
    │   ├── config.py
    │   └── main.py
    ├── console_mqtt_thread/
    │   ├── client_mqtt.py
    │   ├── commandes.py
    │   ├── config.py
    │   ├── main.py
    │   └── thread_mqtt.py
    └── ihm_mqtt_simple/
        ├── client_mqtt.py
        ├── config.py
        ├── dialogue_courbe.py
        ├── graphe.py
        ├── graphe.ui
        ├── main.py
        ├── mon_ihm.py
        ├── mon_ihm.ui
        └── thread_mqtt.py
```

Chaque dossier `SIMULIDE/*/build/arduino.avr.uno/` contient les **cinq fichiers compilés** du croquis correspondant : `.ino.eep`, `.ino.elf`, `.ino.hex`, `.ino.with_bootloader.bin` et `.ino.with_bootloader.hex`. Ils sont déjà suivis par Git dans ce dépôt.

## 1. Simulation avec SimulIDE

La première partie est réalisée avec **Arduino UNO R3** dans SimulIDE. Les circuits sont développés progressivement :

| Dossier | Réalisation |
| --- | --- |
| `SIMULIDE/clignotement_led` | Clignotement d'une LED |
| `SIMULIDE/bargraphe` | Chenillard de dix LED |
| `SIMULIDE/capteur_dht22` | Lecture du DHT22 et commande des LED selon un seuil de 20 °C |
| `SIMULIDE/lcd_i2c` | Premier affichage sur le LCD I2C |
| `SIMULIDE/lcd_i2c_dht22` | Affichage de la température et de l'humidité |
| `SIMULIDE/TempHumiX` | Montage complet avec date, heure, mesures et terminal série |

Pour relancer une simulation :

1. Ouvrir le fichier `.sim1` dans SimulIDE.
2. Charger sur la carte, avec **Load firmware**, le fichier `.ino.hex` correspondant dans `build/arduino.avr.uno/` (sans `with_bootloader`).
3. Démarrer la simulation. Pour le montage complet, le terminal série utilise **9 600 bauds**.

En cas de modification d'un croquis, le recompiler pour **Arduino Uno** dans Arduino IDE, puis recharger le nouveau fichier `.hex` dans SimulIDE.

## 2. Arduino UNO R4 WiFi et communication MQTT

La deuxième partie utilise le **boîtier réel Arduino UNO R4 WiFi**. Les croquis du dossier `TEMPHUMIX` correspondent aux étapes successives :

| Fichier | Étape |
| --- | --- |
| `acquisition/acquisition.ino` | Acquisition des mesures et affichage LCD |
| `rtc/rtc.ino` | Ajout de l'horloge RTC |
| `wifi/wifi.ino` | Connexion Wi-Fi et synchronisation NTP |
| `mqtt/mqtt.ino` | Publication des données via MQTT |
| `commandes/commandes.ino` | Traitement des commandes MQTT reçues |
| `optimisation/optimisation.ino` | Version finale : limitation des publications redondantes |

**Programme final :** [`TEMPHUMIX/optimisation/optimisation.ino`](TEMPHUMIX/optimisation/optimisation.ino).

### Utilisation du programme Arduino

1. Ouvrir `optimisation.ino` dans Arduino IDE et sélectionner **Arduino UNO R4 WiFi**.
2. Installer les bibliothèques nécessaires au croquis (notamment celles du capteur DHT22, du LCD, du MQTT et du NTP).
3. Renseigner **localement** les paramètres Wi-Fi et MQTT dans le fichier de secrets utilisé par le croquis. Ce fichier n'est pas versionné.
4. Téléverser le programme, puis consulter le moniteur série pour vérifier la connexion Wi-Fi, la synchronisation et les messages MQTT.

Paramètres du réseau utilisés pendant les TP : broker Mosquitto `192.168.1.1:1883`, serveur NTP `192.168.1.254`, identifiant du poste/capteur `48763`. Ces adresses appartiennent au **réseau local du TP**.

### Topics MQTT

| Sens | Topic | Donnée / action |
| --- | --- | --- |
| Arduino → broker | `/iot/VERSION_48763` | Version du programme |
| Arduino → broker | `/iot/TEMPERATURE_48763` | Température |
| Arduino → broker | `/iot/HUMIDITY_48763` | Humidité |
| Arduino → broker | `/iot/GET_DATE_48763` | Date et heure |
| PC → Arduino | `/iot/COMMAND_48763` | Code de commande |
| PC → Arduino | `/iot/BLINK_48763` | Période de clignotement en ms |
| PC → Arduino | `/iot/BARGRAPH_48763` | Valeur du bargraphe (0 à 1023) |

Commandes : `10` synchronisation NTP, `20` démarrage des mesures, `30` arrêt des mesures, `40` clignotement lent, `50` clignotement rapide et `60` clignotement personnalisé (avec une période envoyée auparavant sur `BLINK`). **Les noms des topics sont sensibles à la casse.**

Les échanges ont été testés avec **MQTTX** et analysés avec **Wireshark** (publication/souscription, QoS, Retain, Keep Alive).

## 3. Interface Python (PySide6)

La dernière partie se trouve dans `IHM`. Elle comprend trois programmes console successifs et une interface graphique :

- `console_mqtt_lecture` : souscription et affichage des messages reçus.
- `console_mqtt_ecriture` : menu pour envoyer les commandes.
- `console_mqtt_thread` : lecture MQTT avec un thread, en parallèle des commandes.
- `ihm_mqtt_simple` : application PySide6, archivage SQLite et courbes Matplotlib.

L'IHM permet de consulter les mesures du capteur, de synchroniser son horloge, de piloter le clignotement et le bargraphe, puis de retrouver les données enregistrées sur une période choisie.

### Installation sous Windows

Dans PowerShell, depuis la racine du dépôt :

```powershell
cd IHM
py -3.12 -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install -r requirements.txt
```

Python 3.12 est la version indiquée dans le sujet. Si un environnement virtuel est déjà créé, il suffit de l'activer et d'installer les dépendances manquantes.

### Lancer l'interface graphique

Depuis le dossier `IHM` :

```powershell
cd ihm_mqtt_simple
python main.py
```

Renseigner les paramètres du broker dans la fenêtre et cliquer sur **Connecter**. Le boîtier et le PC doivent pouvoir accéder au même broker MQTT.

Le bouton **Afficher les courbes** ouvre l'historique des mesures. La base `IOT.db` est créée localement pendant l'utilisation de l'archivage ; elle n'est pas incluse dans le dépôt. Pour obtenir des courbes, il faut d'abord avoir enregistré des mesures.

### Lancer les programmes console

Depuis le dossier `IHM`, définir les variables MQTT dans chaque terminal PowerShell utilisé :

```powershell
$env:IOT_MQTT_USER = "<utilisateur_MQTT>"
$env:IOT_MQTT_PASSWORD = "<mot_de_passe_MQTT>"
```

Puis exécuter selon l'étape souhaitée :

```powershell
python console_mqtt_lecture/main.py
python console_mqtt_ecriture/main.py
python console_mqtt_thread/main.py
```

Ces programmes sont à lancer **séparément**, sauf pour le test prévu avec deux terminaux distincts (lecture et écriture en parallèle).

## Remarques

- Les fichiers `.ui` sont les formulaires créés avec **Qt Designer** ; `mon_ihm.py` et `graphe.py` sont générés depuis ces formulaires avec `pyside6-uic`.
- Les environnements virtuels, caches Python, fichiers de secrets et bases SQLite locales ne doivent pas être publiés sur GitHub.
- L'accès au broker et certains tests nécessitent le réseau et le boîtier du TP.
- L'historique des commits documente la progression des trois parties.

Projet réalisé dans le cadre des travaux pratiques IoT à **IMT Nord Europe**.
