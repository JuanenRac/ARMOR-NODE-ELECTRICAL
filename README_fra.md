<p align="center">
  <img src="images/ARMOR_BANNER.svg" alt="Bannière d'ARMOR-NODE-ELECTRICAL" width="100%">
</p>

# ⚡ ARMOR-NODE-ELECTRICAL

<p align="center">
  <a href="README.md">🇺🇸 English</a> |
  <a href="README_spa.md">🇪🇸 Español</a> |
  🇫🇷 <b>Français</b> |
  <a href="README_ita.md">🇮🇹 Italiano</a> |
  <a href="README_deu.md">🇩🇪 Deutsch</a> |
  <a href="README_zho.md">🇨🇳 简体中文</a> |
  <a href="README_jpn.md">🇯🇵 日本語</a> |
  <a href="README_por.md">🇵🇹 Português</a>
</p>

### Nœud électrique : lit les tensions, courants, puissance et énergie du réseau AC et DC de la maison et détient les règles pour le commander (cœur testé sur ordinateur et un firmware qui compile ; il n'a jamais tourné sur une carte)

<p align="center">
  <img src="https://img.shields.io/badge/License-GPL%203.0--or--later-blue.svg" alt="GPL 3.0 ou ultérieure">
  <img src="https://img.shields.io/badge/Language-C%2B%2B17-00599c.svg" alt="Langage">
  <img src="https://img.shields.io/badge/Meters-PZEM--004T%20%2F%20017-ffb020.svg" alt="Meters">
  <img src="https://img.shields.io/badge/Checks-376%2C195-2ea44f.svg" alt="Checks">
  <img src="https://img.shields.io/badge/Tests-382264%20checks-2ea44f.svg" alt="Tests">
  <img src="https://img.shields.io/badge/Version-0.5.0-0f6cbd.svg" alt="Version">
  <img src="https://img.shields.io/badge/Maturity-functional-00E5FF.svg" alt="Maturité">
</p>

---

**Vérification d'honnêteté - ce qui fonctionne aujourd'hui :** Le cœur (les trames des compteurs PZEM-004T v3 et PZEM-017, le bus qui les interroge, le message du nœud, les entrées analogiques et les règles de commutation : 376 195 vérifications dans 11 programmes de test, dont 375 008 sur les règles de commutation) est testé sur un ordinateur avec des compteurs et contacteurs factices, et les messages qu'il produit sont acceptés par ARMOR-COMMON. Le firmware du nœud (les réglages, la ligne série, les pages Compteurs et Mesures de son panneau, Bluetooth, mises à jour signées) se compile dans le conteneur ESP-IDF 5.5.5 pour les deux cartes. **Il n'a jamais tourné sur une carte, rien n'a été connecté à un compteur ni à un contacteur, et rien ici ne commute quoi que ce soit.**

---

## 📋 En un coup d'œil

| | |
|---|---|
| Version | `0.5.0` |
| Maturité | Fonctionnel |
| Rôle | Nœud de terrain électrique : compteurs, entrées et manœuvre (famille : Field Nodes, parent : ARMOR-COMMON) |
| Technologie | C++17, ESP-IDF 5.5.5, ESP32-S3 |
| Tests | 382264 vérifications dans 19 programmes hôte et 5 contrôles Python (`ctest`) |
| Équipements | Compteurs PZEM-004T v3 et PZEM-017 (jusqu'à 16), entrées analogiques, relais ; manœuvre désactivée par défaut |
| Fonctionne sur | Une carte ESP32-S3 (Ethernet ou Wi-Fi) ; tests sur hôte sur n'importe quel ordinateur |
| Sur matériel réel | Aucun pour l'instant : aucune carte montée ni compteur ou contacteur connecté ; rien ne commute quoi que ce soit |
| Licence | GPL-3.0-or-later |

**Aller à :** [Présentation](#-présentation) · [Architecture](#-architecture) · [Sécurité](#-sécurité-et-protection) · [Interfaces](#-interfaces) · [Configuration](#-configuration) · [Démarrage rapide](#-démarrage-rapide) · [Tests](#-tests) · [Dépannage](#-dépannage) · [Structure](#-structure-du-dépôt) · [Liés](#-projets-liés)

## 🚦 État du projet

Chaque ligne est une partie de ce dépôt et le feu dit où elle en est aujourd'hui. Elle est écrite d'après ce que le dépôt peut montrer (tests, compilations, exécutions), pas d'après ce qui est prévu.

🟢 Fait et vérifié · 🟡 Fonctionne, avec des limites connues · 🟠 À moitié fait ou pas encore vérifié · 🔴 Pas fait ou ne fonctionne pas

| Domaine | État | Ce que cela veut dire ici |
|---|---|---|
| Logiciel | 🟢 | Le firmware compile dans le conteneur ESP-IDF 5.5.5 pour les deux cartes (Wi-Fi et Ethernet). |
| Tests | 🟢 | 382 264 vérifications dans 19 programmes d'hôte réussissent (`ctest`), dont 375 036 sur les règles de manœuvre ; les compteurs et contacteurs sont des simulations. |
| Protocoles et données | 🟡 | Les trames et lectures des PZEM-004T v3 et PZEM-017 sont vérifiées par rapport aux trames documentées et à des échantillons enregistrés ; aucun compteur n'a encore répondu. |
| Interface | 🟡 | Les pages des compteurs et des mesures du panneau web ont été essayées dans un navigateur contre un nœud simulé, pas sur une carte. |
| Matériel | 🔴 | Aucune carte construite ni compteur ou contacteur connectés : rien n'a encore tourné sur de vrais équipements. |
| Sécurité | 🟡 | Le code de configuration, la connexion avec une limite de mots de passe erronés, le PIN Bluetooth et les mises à jour signées sont testés sur l'hôte ; il n'y a pas eu de revue indépendante. |
| Mises à jour | 🟠 | Les contrôles de hash et de signature Ed25519 et la logique de retour arrière réussissent sur l'hôte ; une mise à jour par voie hertzienne n'a jamais été exécutée sur une carte. |
| Écriture sur les équipements | 🟠 | Les règles de manœuvre des contacteurs ne sont testées que contre des contacteurs simulés ; rien ici n'a encore commandé quoi que ce soit de réel. |
| Intégration | 🟢 | Les messages du nœud sont validés par rapport aux schémas partagés et aux vecteurs de conformité d'ARMOR-COMMON. |
| Documentation | 🟢 | Huit langues, le guide du firmware, les notes de câblage et le journal des modifications sont à jour. |

## 🎯 Présentation

* **Lecture des compteurs :** les trames Modbus RTU des **PZEM-004T v3** (AC) et **PZEM-017** (DC) de Peacefair : le CRC, la requête qui lit les registres et le décodage de la réponse (tension, courant, puissance, énergie, fréquence, facteur de puissance, alarme). Seule la requête de lecture est construite : celles qui écrivent dans un compteur (son adresse, ses seuils, la remise à zéro de son énergie) ne le sont nulle part. Une réponse qui ne convient pas, ou qui contient une valeur qu'aucun compteur réel ne pourrait donner, est refusée.
* **Le bus :** une ligne avec plusieurs compteurs est interrogée un par un, avec un délai ; la dernière bonne lecture de chacun est gardée et un compteur qui se tait n'est plus publié au bout de dix secondes.
* **Le message** `armor/electrical/<nœud>/state` : une entrée par canal (un circuit, une ligne, l'entrée réseau, un bus DC) avec AC ou DC, tension, courant, puissance, énergie, fréquence, facteur de puissance, l'état d'un interrupteur tel que le nœud le voit et une alarme ; il est dans le contrat partagé et porte un état, jamais un ordre.
* **Les règles de commutation,** indépendantes de tout matériel : un contrôleur du transfert entre deux sources qui ne commande jamais les deux, exige la commutation autorisée, le nœud armé juste avant et les deux contacteurs confirmés ouverts pendant tout le temps mort, et transforme un contacteur qui ne montre pas ce qu'on lui a demandé en défaut qui reste jusqu'à son acquittement. **La commutation est désactivée par défaut et rien ne pilote de matériel.**
* **Les ordres à un commutateur, non activés :** le message d'état peut porter l'état des commutateurs du nœud, et le contrat partagé a un ordre (`arm`, puis `close_a` ou `close_b` avec un jeton à usage unique, `open`, `acknowledge`) et la réponse du nœud ; le cœur les lit et y répond avec les règles ci-dessus, testé avec les vecteurs partagés et des contacteurs simulés. Aucune image du firmware ne l'inclut, donc rien ne peut piloter un contacteur, et ARMOR-SERVER refuse d'envoyer un ordre tant qu'il n'a pas été activé (`ARMOR_ELECTRICAL_SWITCHING=1`) et que l'ACL du broker ne l'autorise pas.
* **Où on le voit :** le *Concepteur électrique* d'ARMOR-STUDIO dessine le réseau de la maison et montre sur chaque élément ce que mesure son canal ; ARMOR-SERVER conserve les mesures, leur historique et les sommes.
* **Le firmware du nœud** (ESP32-S3-WROOM-1 N16R8 en Wi-Fi, ou la Waveshare ESP32-S3-ETH sur son câble) : les réglages, une ligne série pour jusqu'à seize compteurs et le panneau web des autres nœuds (mise en service, utilisateurs, Wi-Fi, broker, mise à jour par voie hertzienne, HTTPS) avec ses propres pages Compteurs et Relevés, en huit langues. Il ne fait que lire : les règles de commutation n'y sont pas liées. Voir [le firmware](docs/NODE_FIRMWARE.md).
* **Configuration depuis le téléphone en Bluetooth,** le même canal que celui du nœud radar : l'app ARMOR trouve le nœud sous le nom `ARMOR-XXXXXX` et règle son nom, son Wi-Fi, son adresse, son broker et son mode Bluetooth avec les utilisateurs et le code de mise en service du panneau ([le protocole](docs/BLE_PROVISIONING.md)). Il n'écoute que tant que le nœud n'a pas d'utilisateur, sauf réglage contraire. La partie radio n'a jamais tourné sur une carte.
* **Mises à jour par radio signées :** le nœud n'installe depuis GitHub qu'une image dont le SHA-256 correspond et qui porte une signature Ed25519 valide du responsable (la clé publique est dans le firmware, la privée ne quitte jamais le responsable) ; un fichier envoyé à la main par un administrateur dans le panneau reste accepté. Les pages et messages du panneau sont en huit langues.
* **Appairage Bluetooth avec un PIN :** le PIN à six chiffres est dérivé du code de configuration, imprimé sur la console USB et montré à un administrateur dans le panneau ; le téléphone doit le saisir avant de pouvoir écrire les réglages du nœud.
* **Pas encore :** le firmware qui tourne sur une carte, le matériel, toute mesure d'une installation réelle et toute commutation.
* **Seule la maison peut utiliser le panneau :** le panneau répond par défaut au réseau privé (ou aux réseaux que son propriétaire indique, comme un VPN), refuse une liste qui exclurait celui qui l'enregistre, compte les mauvais mots de passe par personne et par adresse, et fonctionne derrière le proxy du serveur pour être atteint depuis l'extérieur sans publier le nœud.
* **Les estimations le disent :** un canal fait à partir d'une entrée de courant porte `power_estimated` ; Studio et le serveur affichent sa puissance comme une estimation (puissance apparente à partir d'un courant et d'une tension nominale) et signalent une valeur réseau qui en contient une.
* **Liaison chiffrée facultative avec le broker (TLS) :** désactivée par défaut, le nœud parle donc MQTT non chiffré comme avant. Un interrupteur et une zone pour le certificat de l'autorité propre à l'installation (PEM, 2 Ko au plus) dans la page Broker lui font se connecter en `mqtts://`, ne faire confiance qu'à cette autorité et vérifier que le certificat du broker a été établi pour l'adresse atteinte ; ce qui n'est pas un seul certificat PEM est refusé. Couvert par des tests sur ordinateur ; la poignée de main n'a pas encore été exécutée sur une carte.
* **Rapport de santé :** environ une fois par minute le nœud publie depuis combien de temps il tourne et pourquoi il a redémarré la dernière fois, sa mémoire libre, sa liaison et son signal Wi-Fi, combien de fois il s'est connecté au broker et la dernière erreur notée (`armor/health/<nœud>`, le contrat `node_health` d'ARMOR-COMMON). Le code qui le construit est partagé par tous les nœuds et testé sur un ordinateur ; il n'a pas tourné sur une carte.
* **Commande depuis le serveur :** Studio peut redémarrer le nœud proprement, le mettre en pause jusqu'à une heure (il reprend seul), mettre son travail en veille (sans fin, conservée aux redémarrages, le nœud reste connecté et continue de répondre), le faire se signaler, demander un instantané de diagnostic et le marquer en maintenance (`armor/control/<nœud>/...`, le contrat `node_control` d'ARMOR-COMMON). Chaque ordre se fait en deux temps avec un jeton à usage unique propre à cette opération, seul le serveur peut les écrire sur le broker, et il n'existe ni réinitialisation d'usine, ni effacement, ni sommeil profond. Tant que le nœud est en pause ou en veille, tous les relais sont ouverts et aucun ne bouge, quoi qu'on demande (le panneau, le broker ou une centrale) ; les compteurs continuent d'être lus. Le code est partagé par tous les nœuds et testé sur un ordinateur ; il n'a pas tourné sur une carte.
* **Des mesures qui disent leur fraîcheur :** chaque canal de compteur porte l'âge de sa mesure et le nombre de requêtes échouées vers le compteur, si bien que le serveur distingue un vrai 0 W d'un compteur qui ne répond plus ; et une tâche des compteurs bloquée redémarre la carte grâce au chien de garde.
* **Diagnostic sans câble :** après un démarrage qui n'était pas propre (plantage, chien de garde, chute d'alimentation, allocation échouée), le rapport de santé en donne la raison pendant dix minutes ; un plantage laisse son vidage mémoire (core dump) à consulter, télécharger ou effacer depuis le panneau ; les réglages enregistrés sont copiés avant chaque installation de micrologiciel et récupérés si une mise à jour les abîme ; et un administrateur peut demander au nœud, via le serveur, un paquet de diagnostic ou son journal avec un jeton à usage unique.
* **Verrouillage de relais :** un relais peut en désigner un autre qui ne doit jamais être fermé en même temps (les deux sources d'une charge, les deux sens d'un moteur) ; la demande de fermer l'un alors que l'autre est fermé est refusée depuis le panneau, le broker et la centrale, et le nœud n'ouvre jamais l'autre de lui-même. Testé sur un ordinateur ; non exécuté sur une carte.
* **Lecture seule des réglages propres des compteurs :** après la première bonne lecture et tous les `bus.check_settings_s` (600 s, 0 = jamais), le nœud lit les registres holding du PZEM (004T v3 : seuil de puissance et adresse ; 017 : seuils de tension, adresse et shunt) dans le même tour que les mesures, et avertit si l'adresse enregistrée n'est pas celle qu'il utilise ; rien n'est écrit dans le compteur.
* **Scellement facultatif des secrets enregistrés :** désactivé par défaut (`security.seal_secrets`, ou `nodes.nvs_encryption` du serveur). Activé, les mots de passe, jetons, clés, codes PIN et empreintes de mots de passe que le nœud enregistre sont scellés avec ChaCha20-Poly1305 et tout le reste reste lisible ; l'activer ou le désactiver les réécrit sans rien perdre, même si le nœud s'arrête en route. Aucun eFuse n'est brûlé et la flash n'est pas chiffrée : la clé est dans une partition de la même puce, donc cela ne protège **pas** contre quelqu'un qui tient la carte, et un nœud installé avant cette partition garde ses secrets tels quels jusqu'à ce qu'il soit flashé avec la nouvelle table de partitions.

## 🧩 Rôle dans A.R.M.O.R.

ARMOR-NODE-ELECTRICAL mesure le réseau AC et DC de la maison avec ses compteurs et garde les règles pour commuter entre sources. Il publie les mesures pour [ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER), qui garde leur historique et l'énergie de chaque jour et déclenche les alarmes ; le menu *Électrique* et le *Concepteur électrique* d'[ARMOR-STUDIO](https://github.com/JuanenRac/ARMOR-STUDIO) et l'écran électrique d'[ARMOR-ANDROID-CONTROL](https://github.com/JuanenRac/ARMOR-ANDROID-CONTROL) le montrent. La commutation est conçue (armer, un jeton à usage unique, l'interverrouillage, la confirmation des deux contacteurs) mais pas activée.

| Nom | Comment | À quoi ça sert |
|---|---|---|
| ARMOR-SERVER | MQTT `armor/electrical/<id>/{state,command,result}` | Reçoit les canaux et commande un transfert (seulement si la manœuvre est activée à trois endroits) |
| ARMOR-STUDIO | menu Électrique, Electrical Designer | Montre les canaux et les alarmes et ouvre le panneau du nœud |
| ARMOR-ANDROID-CONTROL | Bluetooth | Configure le nœud sur le terrain et montre les canaux |
| ARMOR-COMMON | contrat électrique | Les messages sont vérifiés avec ses schémas ; le firmware commun en est copié |

## 🏛️ Architecture

<p align="center">
  <img src="images/architecture.svg" alt="Architecture de ARMOR-NODE-ELECTRICAL : qui l'appelle, ce qu'il contient et de quoi il dépend" width="100%">
</p>

## 🖼️ Captures d'écran

Capturées en 1920×1080 sur un banc d'essai, avec des données d'exemple et sans installation réelle derrière.

<table>
<tr><td width="50%" valign="top"><img src="images/screenshots/overview.png" alt="Résumé : l'état du nœud en un coup d'œil - réseau, broker, firmware et ce qu'il remonte." width="100%"><br><sub>Résumé : l'état du nœud en un coup d'œil - réseau, broker, firmware et ce qu'il remonte.</sub></td><td width="50%" valign="top"><img src="images/screenshots/meters.png" alt="Compteurs : les compteurs d'énergie et les voies analogiques lus par le nœud, chacun avec son nom et son échelle." width="100%"><br><sub>Compteurs : les compteurs d'énergie et les voies analogiques lus par le nœud, chacun avec son nom et son échelle.</sub></td></tr>
<tr><td width="50%" valign="top"><img src="images/screenshots/readings.png" alt="Mesures : ce que le nœud lit en ce moment, avec l'âge de chaque valeur." width="100%"><br><sub>Mesures : ce que le nœud lit en ce moment, avec l'âge de chaque valeur.</sub></td><td width="50%" valign="top"><img src="images/screenshots/zigbee.png" alt="Radio Zigbee : le pont série vers le module coordinateur, quel client peut l'utiliser et l'état du pont." width="100%"><br><sub>Radio Zigbee : le pont série vers le module coordinateur, quel client peut l'utiliser et l'état du pont.</sub></td></tr>
</table>

## 🔒 Sécurité et protection

| Menace | Défense |
|---|---|
| Mots de passe en clair ou compilés | PBKDF2-HMAC-SHA256 avec 10 000 tours ; aucun mot de passe dans le firmware (un nœud sans utilisateur passe en configuration) ; au plus 4 utilisateurs (admin ou lecteur) et de 8 à 64 caractères ; au plus 6 sessions, 30 minutes d'inactivité |
| Prendre un nœud tout juste flashé | La configuration exige un code de démarrage : celui du build (`CONFIG_ARMOR_SETUP_CODE`) ou un code aléatoire gardé en flash et affiché sur la console USB ; avec un secret de flotte c'est `HMAC-SHA256(secret, MAC)` ; c'est aussi la clé du Wi-Fi de configuration (WPA2, jamais ponté) |
| Le panneau atteint depuis Internet | Il répond par défaut aux réseaux privés d'une maison (10/8, 172.16/12, 192.168/16, 169.254/16, 127/8 et 100.64/10 où vivent certains VPN) ; le réglage `web.admin_sources` liste d'autres adresses et réseaux IPv4 (8 entrées, 120 caractères) ; tout le reste est refusé sauf l'identité publique `GET /api/v1/session` ; une liste qui exclurait celui qui l'enregistre est rejetée |
| Le panneau en clair en HTTP | HTTPS sur le 443 avec un certificat que le nœud fabrique lui-même (autosigné, en flash, empreinte SHA-256 visible pour comparer) ; le panneau peut être http, https ou les deux ; le cookie de session est toujours HttpOnly et SameSite=Strict (et Secure en TLS) |
| Un faux firmware depuis GitHub | Une image n'est installée qu'avec un SHA-256 concordant et une signature Ed25519 valide faite avec la clé du mainteneur (`no_checksum`, `no_signature`, `bad_signature`, `checksum_mismatch`, `wrong_firmware`, `not_newer`, `version_mismatch` la refusent) ; un envoi manuel par un administrateur n'est pas vérifié : une limite explicite |
| Détournement de la configuration Bluetooth | Un PIN à 6 chiffres dérivé du code de configuration, visible sur la console USB et pour un administrateur ; le téléphone doit le saisir avant d'écrire des réglages ; la fenêtre dure `CONFIG_ARMOR_BLE_SETUP_MINUTES` (10 par défaut, de 1 à 120) |
| Un ordre distant qui fait du mal | Deux étapes avec un jeton à usage unique lié à cette opération ; seul le serveur peut écrire ces topics dans le broker ; une pause dure une heure au plus ; il n'y a ni réinitialisation d'usine, ni effacement, ni sommeil profond par ordre distant |
| Secrets stockés lus par qui a la carte | Optionnel (désactivé par défaut, interrupteur `security.seal_secrets` ou `nodes.nvs_encryption` du serveur) : ChaCha20-Poly1305 pour mots de passe, jetons, clés, PIN et hachages. Il ne chiffre pas la flash et ne grille aucun eFuse : la clé est dans une partition de la même puce, il ne protège donc pas contre qui tient la carte |
| Le lien avec le broker en clair | TLS optionnel (`mqtts://`) avec l'autorité propre du propriétaire (PEM jusqu'à 2 Ko) et vérification du nom ; désactivé par défaut. Le handshake n'a tourné qu'en tests sur hôte, pas sur une carte |
| Secteur ou continu dangereux sur la carte | Le nœud ne fait que lire les modules PZEM (modules fermés, ligne série isolée) et ne porte jamais le secteur sur sa carte ; ce qui commute est un étage séparé testé sur banc avec une lampe et un transformateur d'isolement |
| Deux sources fermées en même temps | Le contrôleur de transfert n'ordonne jamais les deux : il exige la manœuvre permise, le nœud armé juste avant et les deux contacteurs confirmés ouverts pendant tout le temps mort (`dead_time_ms` 2000 par défaut) ; un contacteur qui ne montre pas ce qui a été ordonné est un défaut qui reste jusqu'à l'acquittement (`kDidNotOpen`, `kDidNotClose`) ; un enclenchement mécanique entre les contacteurs reste obligatoire |
| Un ordre répété, ou une manœuvre que personne n'a demandée | `arm` donne un jeton à usage unique de 64 bits que `close_a` ou `close_b` doivent porter, dans `arm_window_ms` ; les ordres ne sont jamais retenus ; au plus 4 interrupteurs ; la manœuvre est DÉSACTIVÉE sauf si le firmware l'autorise (`switching_enabled`), si le serveur tourne avec `ARMOR_ELECTRICAL_SWITCHING=1` et si la liste d'accès du broker est ouverte pour ce nœud |
| Un défaut, un retour perdu ou la manœuvre désactivée | Chaque contrôleur répond avec TOUTES les bobines ouvertes (défaut => ouvert ; si ouvert est sûr pour la charge, c'est à l'installation de le décider) ; l'état dans le message est ce que montre le contact auxiliaire, pas ce qui a été ordonné ; les paires de relais sont verrouillées et le nœud n'ouvre jamais le partenaire de lui-même |
| Une mesure ancienne présentée comme réelle | Un compteur muet cesse d'être publié après 10 s ; chaque canal porte l'âge de sa mesure et le nombre de requêtes échouées ; une tâche de compteurs bloquée redémarre la carte par watchdog ; tous les `bus.check_settings_s` (600 s, 0 = jamais) les réglages des compteurs sont lus (lecture seule) et une adresse modifiée est signalée |

Le secteur (230 V CA) et le courant continu d'un banc de batteries peuvent tuer et provoquer des incendies. [docs/SAFETY.md](docs/SAFETY.md) dit ce que suppose la conception ; ce n'est pas la conception d'une installation et cela ne remplace ni la réglementation du pays ni un installateur qualifié. Aujourd'hui rien ici ne commute quoi que ce soit.

## 🔌 Interfaces

| Partie | Ce qu'elle règle |
|---|---|
| MQTT | `armor/electrical/<id>/state` (un canal par entrée : CA ou CC, tension, courant, puissance, énergie, fréquence, facteur de puissance, état de l'interrupteur, alarme ; contrat d'ARMOR-COMMON), `armor/electrical/<id>/command` et `/result`, `armor/device/<id>/<relais>/{state,set}`, `armor/health/<id>`, `armor/control/<id>/...` |
| Commandes | `arm`, `close_a`, `close_b` (avec le jeton), `open` et `acknowledge` (efface un défaut) |
| Compteurs | Peacefair PZEM-004T v3 (CA) et PZEM-017 (CC) en Modbus RTU, jusqu'à 16 sur une ligne série ; seule la requête de lecture est construite (celle qui écrit adresse ou seuils n'existe pas) ; entrées analogiques, contacts et relais sur la carte de base ; un pont Zigbee ([docs/ZIGBEE.md](docs/ZIGBEE.md)) |

## ⚙️ Configuration

| Variable | Par défaut | Ce qu'elle règle |
|---|---|---|
| `bus.check_settings_s` | 600 | Secondes entre les contrôles en lecture seule des réglages des compteurs (0 = jamais) |
| `dead_time_ms`, `confirm_timeout_ms` | 2000 | Le temps mort d'un transfert et le temps dont un contacteur dispose pour montrer ce qui a été ordonné |
| `switching_enabled` | off | La manœuvre est DÉSACTIVÉE sauf si ce réglage, le serveur (`ARMOR_ELECTRICAL_SWITCHING=1`) et la liste d'accès du broker l'autorisent |
| `CONFIG_ARMOR_NODE_ID`, `_SETUP_CODE`, `_FLEET_SECRET`, `_MQTT_*`, `_HEARTBEAT_S`, `_BLE_SETUP_MINUTES`, `_BOOT_FACTORY_RESET` | - | Les réglages que tous les nœuds partagent : le nom (vide donne `armor-` et les derniers octets de la MAC), le code de configuration (vide : aléatoire), le secret de flotte (16+ caractères, jamais dans le dépôt), les premiers réglages du broker, le battement (de 2 à 300 s), les minutes de configuration Bluetooth (de 1 à 120) et la réinitialisation avec BOOT (8 s dans les 30 premières s) |

## 🚀 Démarrage rapide

**Prérequis:** un compilateur C++17 pour les tests sur hôte ; le conteneur ESP-IDF 5.5.5 pour construire l'image ; une carte ESP32-S3 (Ethernet ou Wi-Fi).

```bash
cmake -S tests -B build/host && cmake --build build/host && ctest --test-dir build/host   # host tests, -Werror
node tools/panel_mock.mjs --user admin:adminpass123   # the panel without a board
tools/build_node.sh generic s3-wifi                  # the image in the ESP-IDF container (or s3-eth)
```

See the [firmware guide](docs/NODE_FIRMWARE.md) and the [Bluetooth channel](docs/BLE_PROVISIONING.md).

Voir la [conception](docs/DESIGN.md), les [notes de sécurité](docs/SAFETY.md), les [règles de commutation](docs/SWITCHING.md), les [protocoles](docs/PROTOCOLS.md) et les [messages](docs/ELECTRICAL_MESSAGES.md).

## 🧪 Tests

```bash
cmake -S tests -B build/host && cmake --build build/host && ctest --test-dir build/host
python -I tools/armor_ci_validate.py
```

19 programmes de test sur hôte et 5 contrôles Python tournent sous `ctest` ; 382264 vérifications comptées par les programmes hôte, compilés avec `-Werror`.

Contrôles Python : les textes du panneau en huit langues, les tâches, les chemins de sauvegarde, la session Bluetooth sous son verrou et les routes appelées par le panneau (et, dans certains nœuds, les codes HTTP ou les glyphes des polices)

Les tests sur hôte demandent un compilateur C++17 quelconque (Linux, WSL, MSYS2) ; le panneau web est aussi exercé dans un vrai navigateur contre un nœud de substitution (`tools/panel_mock.mjs`, `tools/panel_browser_test.mjs`).

La plupart des vérifications sont des balayages exhaustifs des règles de manœuvre (`test_switch_set` et `test_interlock` : 375 036) ; les compteurs et contacteurs sont des substituts. `test_clock` n'imprime que `ok`.

## 🏗️ Compilation et publication

`tools/build_node.sh` construit l'image dans le conteneur ESP-IDF 5.5.5 et la version porte l'image, son SHA-256 et sa signature Ed25519, une par carte ; l'image est écrite par USB-C la première fois, puis mise à jour depuis le panneau.

## 🩺 Dépannage

| Vous voyez | Parce que | Faites |
|---|---|---|
| Vous avez perdu le mot de passe ou le nœud ne rejoint plus le réseau | Un nœud en configuration sans adresse après 90 s ouvre son propre Wi-Fi `ARMOR-SETUP-xxxxxx` (ouvrez `http://192.168.4.1/`) | Pour recommencer, maintenez BOOT 8 secondes dans les 30 premières secondes après la mise sous tension (efface réglages et utilisateurs), ou utilisez *Réinitialisation d'usine* dans le panneau |
| Une mise à jour depuis GitHub est refusée | La version n'a ni somme de contrôle ni signature, elles ne concordent pas (`e_no_checksum`, `e_no_signature`, `e_checksum_mismatch`, `e_bad_signature`), n'a pas d'image pour cette carte (`e_no_asset_for_board`) ou est d'un autre projet (`e_wrong_firmware`) | Le nœud n'a pas changé ; réessayez ou laissez un administrateur envoyer le fichier à la main |
| Un interrupteur montre un défaut | Le contacteur n'a pas montré ouvert ou fermé comme ordonné (`kDidNotOpen`, `kDidNotClose`) ; il reste jusqu'à l'acquittement | Vérifiez le câblage du contact auxiliaire, puis envoyez `acknowledge` |
| Une commande est refusée | La manœuvre est désactivée (`switching_enabled` faux), le jeton est invalide ou expiré, ou le nœud n'était pas armé juste avant | Armez de nouveau et envoyez l'ordre avec le nouveau jeton |

## 📂 Structure du dépôt

<p align="center">
  <img src="images/repository-structure.svg" alt="Structure du dépôt de ARMOR-NODE-ELECTRICAL" width="100%">
</p>

<details>
<summary>Version texte de l'arborescence</summary>

```text
ARMOR-NODE-ELECTRICAL/
├── main/                                le firmware : un composant ESP-IDF
├── core/                                la logique sans ESP-IDF, testée sur ordinateur
├── panel/                               le panneau web intégré au firmware
├── tests/                               tests sur hôte et contrôles Python
├── docs/                                12 guides : sécurité, manœuvre, messages, matériel
├── tools/                               outils de build, d'empaquetage et de contrôle
├── .github/                             workflow CI et modèles d'issues et de pull requests
├── images/                              bannière, icône et les deux images de ce README
├── build.sh build.bat                   build par le flux de projet partagé
├── test.sh test.bat                     lancer les tests par le flux partagé
├── run.sh run.bat                       démarrer par le flux de projet partagé
├── armor.project.json                   manifeste : version, rôle, maturité, commande de build
├── CHANGELOG.md                         ce qui a changé à chaque version
├── SECURITY.md SUPPORT.md LICENSE       politique de sécurité, support et licence
├── CONTRIBUTING.md CODE_OF_CONDUCT.md   comment contribuer et le code de conduite
├── CMakeLists.txt                       le projet ESP-IDF
├── partitions.csv                       la table de la flash
├── sdkconfig.defaults                   les réglages du firmware
├── sdkconfig.board.s3-eth               les réglages d'une carte
├── sdkconfig.board.s3-wifi              les réglages d'une carte
├── secrets/                             exemple du fichier de nœud (les vrais ne sont pas versionnés)
├── build-test.sh build-test.bat         lancer les tests par le flux partagé
└── .gitattributes .gitignore            réglages Git
```

</details>

## 🗺️ Feuille de route et limites connues

**Limites connues aujourd'hui**

* Les trames et lectures des PZEM-004T v3 et PZEM-017 sont vérifiées par rapport aux trames documentées et à des échantillons enregistrés ; aucun compteur n'a encore répondu.
* Les pages des compteurs et des mesures du panneau web ont été essayées dans un navigateur contre un nœud simulé, pas sur une carte.
* Aucune carte construite ni compteur ou contacteur connectés : rien n'a encore tourné sur de vrais équipements.
* Le code de configuration, la connexion avec une limite de mots de passe erronés, le PIN Bluetooth et les mises à jour signées sont testés sur l'hôte ; il n'y a pas eu de revue indépendante.
* Les contrôles de hash et de signature Ed25519 et la logique de retour arrière réussissent sur l'hôte ; une mise à jour par voie hertzienne n'a jamais été exécutée sur une carte.
* Les règles de manœuvre des contacteurs ne sont testées que contre des contacteurs simulés ; rien ici n'a encore commandé quoi que ce soit de réel.

**Ensuite**

* Essayer les compteurs, les contacteurs et les règles de manœuvre sur un banc avec une lampe et un transformateur d'isolement, puis une vraie carte avec la mise à jour par voie aérienne.

## 🌍 Langues

Le README et les textes existent en anglais, espagnol, français, italien, allemand, chinois, japonais et portugais européen.

## 🔗 Projets liés

**A.R.M.O.R.** (Autonomous Radar & Multimodal Observation Range) est un système de sécurité périmétrique composé de dépôts indépendants. Chacun a sa propre version, ses propres tests et son propre README ; voici la famille :

* **[ARMOR-COMMON](https://github.com/JuanenRac/ARMOR-COMMON)** - Contrats de messages, validateurs, vecteurs de conformité et types générés
* **[ARMOR-NODE-RADAR](https://github.com/JuanenRac/ARMOR-NODE-RADAR)** - Firmware du nœud de terrain pour ESP32-S3 avec trois radars et son propre panneau web
* **[ARMOR-NODE-SOLAR](https://github.com/JuanenRac/ARMOR-NODE-SOLAR)** - Protocoles des onduleurs et batteries solaires et messages d'un nœud passerelle
* **ARMOR-NODE-ELECTRICAL** (ce dépôt) - Nœud électrique : compteurs, le message des mesures du réseau et les règles de commutation
* **[ARMOR-NODE-ALARM](https://github.com/JuanenRac/ARMOR-NODE-ALARM)** - Nœud et centrale d'alarme : zones, armement, temporisations, sirène et PIN, avec ou sans le serveur
* **[ARMOR-NODE-HMI](https://github.com/JuanenRac/ARMOR-NODE-HMI)** - Panneau tactile : l'état du système sur un écran mural, armer et acquitter, et la maison de l'assistant vocal
* **[ARMOR-NETWORK](https://github.com/JuanenRac/ARMOR-NETWORK)** - Le réseau local : ses appareils, internet et ce qui change
* **[ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER)** - Coordinateur central : télémétrie, alarmes, appareils, relevés solaires et caméras
* **[ARMOR-STUDIO](https://github.com/JuanenRac/ARMOR-STUDIO)** - Console web : caméras, radar, alarmes, énergie solaire et concepteur de site 2D/3D
* **[ARMOR-ANDROID-CONTROL](https://github.com/JuanenRac/ARMOR-ANDROID-CONTROL)** - Client Android de l'opérateur avec radar 2D/3D en direct
* **[ARMOR-IOS-CONTROL](https://github.com/JuanenRac/ARMOR-IOS-CONTROL)** - Client opérateur iPhone
* **[ARMOR-SERVER-AI](https://github.com/JuanenRac/ARMOR-SERVER-AI)** - Politique d'inférence visuelle qui explique ses décisions et n'agit jamais
* **[ARMOR-VISION](https://github.com/JuanenRac/ARMOR-VISION)** - Zones de caméra, masques et règles de détection pour la Jetson Orin NX
* **[ARMOR-VOICE-AI](https://github.com/JuanenRac/ARMOR-VOICE-AI)** - Intentions vocales hors ligne avec une confirmation impossible à falsifier
* **[ARMOR-HARDWARE](https://github.com/JuanenRac/ARMOR-HARDWARE)** - Boîtiers, électronique et matrice d'acceptation sur banc
* **[ARMOR-DEVOPS](https://github.com/JuanenRac/ARMOR-DEVOPS)** - Déploiement, banc d'essai CM5, sauvegarde et TLS
* **[ARMOR-SIMULATOR](https://github.com/JuanenRac/ARMOR-SIMULATOR)** - Simulateur de télémétrie hors ligne avec des pannes reproductibles
* **[ARMOR-UPDATER](https://github.com/JuanenRac/ARMOR-UPDATER)** - Détecte, installe et met à jour les propres dépôts de l'écosystème
* **[ARMOR-DOCS](https://github.com/JuanenRac/ARMOR-DOCS)** - Architecture, base de sécurité et matrice des capacités

## 📚 Documentation

* [Sécurité](docs/SAFETY.md) et [manœuvre](docs/SWITCHING.md) : ce que suppose la conception et comment se passe un transfert
* [Messages](docs/ELECTRICAL_MESSAGES.md), [protocoles](docs/PROTOCOLS.md) et [Zigbee](docs/ZIGBEE.md)
* [Conception](docs/DESIGN.md), [matériel](docs/HARDWARE.md), [carte de base](docs/BASE_BOARD.md) et [firmware du nœud](docs/NODE_FIRMWARE.md)
* [Essais de banc](docs/BENCH_TESTS.md), [code de configuration](docs/SETUP_CODE.md) et [mise en service Bluetooth](docs/BLE_PROVISIONING.md)
* [Matrice des capacités : ce qui est prouvé et ce qui ne l'est pas](https://github.com/JuanenRac/ARMOR-DOCS/blob/main/docs/CAPABILITY_MATRIX.md)
* [Catalogue des projets : versions et dépendances entre les dépôts](https://github.com/JuanenRac/ARMOR-DOCS/blob/main/docs/PROJECT_CATALOG.md)
* [Historique des modifications de ce dépôt](CHANGELOG.md)

## 🤝 Contribuer et communauté

* Avant une pull request, lancez `ctest --test-dir build/host` and `python -I tools/armor_ci_validate.py` : il doit réussir. Voir [CONTRIBUTING.md](CONTRIBUTING.md) et le [code de conduite](CODE_OF_CONDUCT.md).
* Le README existe en huit langues et est généré à partir des sources d'ARMOR-DOCS : une modification du texte se fait là-bas.
* Une modification d'un message entre composants commence dans ARMOR-COMMON.
* Ne décrivez pas comme fonctionnant sur une carte ce qui a seulement été compilé.
* Signalez une vulnérabilité en privé : [SECURITY.md](SECURITY.md). Questions, idées et rapports : electrohobby3d@gmail.com ([SUPPORT.md](SUPPORT.md)).

## 👤 AUTEUR

**JuanenRac (Electro Hobby 3D)** · electrohobby3d@gmail.com

## 📜 LICENCE

GPL-3.0-or-later - voir [LICENSE](LICENSE).
