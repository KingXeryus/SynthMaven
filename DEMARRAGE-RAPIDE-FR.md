# SynthMaven 1.0 — démarrage rapide

**23 moteurs et 537 presets d’usine**, dont 41 voix FM6, neuf sons compatibles XD
et 88 presets mélodiques orientaux / Moyen-Orient. Les essais d’écoute sur FM1,
la connexion/reconnexion Chocolate et l’audio sous charge Bluetooth restent à faire.

## Installation

Dans Chrome ou Edge, connecte la FM1 en USB et sauvegarde-la avec
[l’éditeur](https://kingxeryus.github.io/SynthMaven/webapp/editor/). Lance ensuite
[l’installateur](https://kingxeryus.github.io/SynthMaven/webapp/installer/).
Le démarrage et About indiquent SynthMaven 1.0.

## Presets

SAVE → PRESETS affiche quatre noms. KNOB 1 sélectionne ; KNOB 2 filtre par
moteur, USER, ORIENTAL ou ALL ; KNOB 3 charge ; KNOB 4 charge et ouvre EDIT.
ORIENTAL contient 44 instruments mélodiques en variantes Dry et Expressive
(88 presets, suffixes D/E). Aucune percussion ajoutée. Les envois chorus, delay
et réverb sont à zéro ; ajoute les effets avec FX. Sauvegarde tes modifications
sous USER. L’éditeur affiche les noms complets. L’accordage est indépendant :
utilise Scala ou l’accordage personnalisé pour le maqam souhaité.

## Connexions MIDI

GLO → KEY MIDI → KEY IN : USB, TRS ou BOTH. GLO → PATCH MIDI → PROG active ou
désactive les changements de son ; PORT choisit USB, BLE ou ANY. Pour USER,
envoie CC0=127 puis Program Change 0 pour U01, 1 pour U02, etc.

GLO → BLUETOOTH : BT MIDI, SCAN, sélection PEDAL, puis LINK. MIDI READY signifie
que le service MIDI est prêt. Les deux modèles Chocolate restent à valider sur
le matériel. USB est actuellement en mode périphérique : un hôte MIDI est requis.
Un clavier alimenté séparément ne remplace pas cet hôte. TRS clavier + BLE pédale
est la combinaison autonome prévue.

Voir le [catalogue et les notes 1.0](docs/release-1.0.md), le
[guide anglais](QUICKSTART.md), [l’état USB](docs/USB_MIDI_STATUS.md) et
[l’état Bluetooth](docs/BLUETOOTH_STATUS.md).
