# Clipper — démarrage rapide (version de test 0.9)

**Clipper** est un firmware de **synthétiseur** pour le M-VAVE FM-1 : un seul instrument, comme le firmware d'origine, Felucca ou Baudgirl. Ce n'est plus une groovebox : pas de pistes, pas de boîte à rythmes, pas de mode song, pas de samples intégrés. Il garde de SLOOP les 12 moteurs, les effets, l'arpégiateur, le séquenceur de pas, l'audio USB et le MIDI. Il ajoute **HYBRID**, une voix à 3 oscillateurs, 2 filtres et une matrice de modulation de 16 slots, avec 24 sons d'usine. Il ajoute aussi le pitch bend, la molette, l'aftertouch et la pédale de sustain sur tous les moteurs.

> **C'est une première version de test** : elle n'a encore jamais tourné sur une vraie FM-1. Avant d'installer, fais une **sauvegarde** dans l'éditeur SLOOP (Projects → Backup → *Save a backup*).

## Un dossier à part

Clipper vit dans son propre dossier, **`CLIPPER`**, à côté de celui de SLOOP. Rien n'est partagé entre les deux :

| | SLOOP | Clipper |
| --- | --- | --- |
| Lancer l'installation | `INSTALL-SLOOP.bat` | `INSTALL-CLIPPER.bat` |
| Ouvrir l'éditeur | `OPEN-EDITOR.bat` | `OPEN-EDITOR.bat` (du dossier CLIPPER) |
| Adresse | http://localhost:**8766** | http://localhost:**8767** |
| Écran de démarrage | voilier, « sloop » | trois-mâts, « clipper » |

Sur la FM-1, Clipper se reconnaît à son écran de démarrage et à HOME (maintenu) → SYS → ABOUT, qui affiche **CLIPPER 0.9**.

## 1. Compiler et installer

1. Ferme la fenêtre de SLOOP si elle tourne encore. Ce n'est pas obligatoire, mais c'est plus clair.
2. Double-clique sur **`INSTALL-CLIPPER.bat`**. Il utilise les mêmes outils que SLOOP (WSL MinUI-Build, la chaîne JieLi). La première fois, il copie les trois fichiers du SDK depuis `..\SLOOP\build\deps\ac79`. S'il ne les trouve pas, copie ce dossier `build\deps\ac79` de SLOOP dans `CLIPPER\build\deps\`.
3. La page d'installation s'ouvre sur http://localhost:8767/webapp/installer/ (Chrome ou Edge, FM-1 en USB). Clique sur **Install**.
4. La FM-1 redémarre sur l'écran **clipper**. Tes projets, presets, samples et réglages de SLOOP sont toujours là.

## Nouveau dans la 0.9 : modèles de filtre, VINTAGE, micro-accordage, une machine de Turing, un son propre

- **Modèles de filtre** (FILTER → **MODL**, sur ANALOG, WAVE et VECTOR ; HYBRID garde ses deux filtres) : **SVF** (12 dB, comme avant), **MOOG** (le ladder 24 dB), **MS20** (2 pôles avec l'écrêtage à diodes dans la réinjection : il hurle à forte résonance sans décrocher), **DIODE** (le ladder à diodes 4 pôles, famille TB-303 / EMS : caoutchouteux, perd moins de basses), **SEM** (le state variable 12 dB d'Oberheim, résonance douce), **NOTCH**, **BP** et **HP**. Changer de modèle sur une note tenue fait un petit creux puis revient (pas de clic).
- **VINTAGE** (VOICE 2 → **VINTG**, tous les moteurs) : chaque voix a sa propre petite erreur d'accord et de filtre (sa « carte de voix »), une lente dérive de hauteur, et chaque note un peu de la sienne. À 127 : carte ±6 cents, dérive ±5, note ±2, coupure ±1,5 pas. À 0 : identique à avant, au bit près.
- **TUNING** (GLO → **TUNING**, pour tous les sons, gardé avec les réglages) : **EQUAL**, **JUST** (intonation juste), **PYTHA**gore, **MEANT**one (1/4 de comma), **WERCK**meister III, **KIRNB**erger III, les maqams **RAST** et **BAYAT**i (tierces neutres), **SLEND**ro et **PELOG** (gamelan), **19**, **24** et **31-EDO**, et **USER** : n'importe quelle gamme **Scala .scl** (jusqu'à 48 notes par période, n'importe quelle période, même la tritave de Bohlen-Pierce) envoyée depuis l'éditeur web. ROOT choisit la tonique.
- **TURNG**, une machine de Turing dans l'arpégiateur (ARP MODE → TURNG, d'après la Turing Machine de Music Thing) : un registre à décalage en boucle choisit parmi les notes tenues. Sur la page SPICE : **MUTAT** (0 figé, 127 aléatoire), **SEED** (un autre motif), **LOOP** (longueur 1..16). LOOP fixe aussi sur combien de notes SPICE / DICE varient avant de se répéter.
- **Un son propre, sans clics** (un nouveau test de 45 cas rend chaque changement et vérifie qu'aucun échantillon ne saute) : le limiteur de sortie bouge en courbe en S ; les envois (CHOR / DLY / REV) et la DIST de l'INSERT glissent au lieu de sauter ; le TIME du delay passe en fondu de l'ancienne à la nouvelle prise ; la DEPTH du chorus glisse ; le FILT du master passe par le milieu entre LP et HP ; FLANG / CHOR de l'INSERT attendent que leur ligne soit pleine ; une table WAVE, un patch FMX ou un modèle de filtre changés sur une note tenue font un creux et reviennent.
- **3 nouveaux sons** (TURING JUNO, MS20 VECTOR, DIODE VS BASS). **395 sons, 23 moteurs.**
- **Corrigé** : un projet SLOOP 2.6 chargé dans Clipper lisait ses valeurs MOD SEQ / SPICE au-delà de celles de son INSERT (maintenant leurs valeurs par défaut).
- **L'éditeur web** (protocole 14) : une carte **Tuning** (système, tonique, charger un .scl) et les nouveaux réglages.

## Nouveau dans la 0.8 : JUNO et son chorus, VECTOR, un séquenceur de modulation, SPICE / DICE

- **JUNO**, le polyphonique des Roland Juno-6 / 60 / 106 (l'idée, aucun code de Roland) : un DCO par voix avec une **dent de scie**, une **impulsion** dont le LFO fait varier la largeur (PWM : LFO DEST SHP, la molette, SHAPE dans la matrice) ou **les deux** (**S+P**, les deux boutons du 106), un **sub** carré une octave en dessous et du **bruit** ; le **HPF** aux quatre positions du 106 (0 un peu de basses en plus, 1 neutre, 2 et 3 plus fin), le **filtre ladder 24 dB** résonant, et le **chorus** : **I** (0,5 Hz, large), **II** (0,8 Hz, plus profond), **I+II** (8 Hz, léger, les deux boutons du 60 ensemble) : un chorus BBD stéréo, deux prises d'une même ligne balayées en sens opposés, assombries comme un BBD, gauche = son sec + l'une, droite = son sec + l'autre. Changer de mode pendant un accord tenu se fait en fondu (pas de clic). EDIT : WAVE PW SUB NOIS / HPF CUT RES CHOR. 16 sons (nappes, cuivres, cordes, basses, claviers, lead synthwave, un hoover).
- **Le chorus du JUNO sur n'importe quel son** : l'INSERT a trois types de plus, **JUNO1**, **JUNO2**, **JUN12** (A WIDTH, B TONE, C NOISE : le souffle du BBD ; MIX le chorus).
- **VECTOR**, la synthèse vectorielle du Sequential Prophet VS et du joystick du Wavestation : quatre formes d'onde aux coins d'un carré (**A B C D**, 16 formes de Clipper, chacune en trois copies sans repliement), un point dans le carré (**X**, **Y**) les mélange, et **MOVE** fait voyager le point tout seul dès le début de chaque note : CIRC, 8, SWEEP (de A jusqu'au point : l'enveloppe de mixage du VS), LOOP (A B D C), RND, chacun lent ou rapide (+). SHAPE déplace X ; la matrice déplace X et Y par leur nom. Puis un passe-bas 12 dB. EDIT : A B C D / X Y MOVE CUT. 16 sons.
- **MOD SEQ**, un séquenceur de modulation dans chaque son (LFO → **MOD SEQ**, **MOD SEQ 2**) : quatre lignes de 16 pas (-8..7) qui avancent à **RATE** (1/32 à 2 mesures, au tempo), sur **LEN** pas, chaque ligne avec son lissage (**SMTH**), à partir de chaque première touche (**START KEY**) ou sur la grille du morceau (**SYNC**). La matrice les lit comme quatre sources de plus, **SEQ1..SEQ4** : une ligne sur le cutoff, une autre sur la hauteur, un rythme sur le volume, n'importe quel réglage EDIT par son nom. Sur la FM-1 : LANE STEP VALUE et **SHAPE** (RAMP+ RAMP- TRI SINE SQR RND FLAT INVRT SHIFT : tourne pour écouter chaque forme), la ligne dessinée en grand, le pas joué allumé, les quatre lignes en petit dessous. Un son chargé apporte quatre formes de départ (une rampe, un triangle, un rythme, une ligne aléatoire). Gardé avec le son : UNDO, COMPARE, le SWAP du layer, les presets utilisateur, les projets.
- **SPICE / DICE** (ARP → **SPICE**, comme sur l'Arturia MicroFreak) : les notes de l'arpégiateur et les pas du séquenceur varient autant que SPICE le dit : certaines se taisent, d'autres sautent d'une octave, la vélocité et la durée bougent ; toujours de la même façon à chaque tour (16 notes, ou les pas du motif), jusqu'à ce qu'on tourne **DICE** (une autre variation). Ce qui est enregistré ne change jamais. SPICE à 0 : exactement comme avant.
- **L'éditeur web** (protocole 13) : une carte **MOD SEQ** (les quatre lignes en barres à tirer, les formes, LIVE), les nouvelles pages (MOD SEQ 2, SPICE, les types JUNO de l'INSERT), JUNO et VECTOR.
- **Les deux nouveaux moteurs tournent sur les deux cœurs** (au bit près, comme les autres). **392 sons, 23 moteurs.**
- **Corrigé** : un preset utilisateur envoyé depuis l'éditeur web était écrit au format de SLOOP par-dessus l'enregistrement de Clipper (LFO 2, ENV 2, matrice et INSERT perdus, et quelques octets écrits après l'enregistrement).

## Nouveau dans la 0.7 : les deux cœurs, l'historique du son, et le meilleur de SLOOP 2.7

- **Le 2e cœur de la FM-1 calcule la moitié des voix.** Le démarrage du 2e cœur vient de X0X (Charles Vestal), via SLOOP 2.7 (Andre Rezemini), avec le correctif de la 2.7.1. SLOOP partage par piste ; Clipper joue un seul son, donc il partage **les voix d'un même son** : le 1er cœur calcule les modulations, donne la 2e moitié des voix au 2e cœur, calcule la 1re moitié, attend et additionne. Le son est identique au bit près (prouvé par les tests sur ordinateur pour chaque preset). Moteurs partagés : HYBRID, FMX, WAVE, ANALOG, FM6, VPS, TERRAIN, WSEQ, MACRO, STRINGS, TRIO, PHASE, DIGITAL, LOFI. **MENU → SYS → 2ND CORE** ON / OFF ; ABOUT affiche **2 CORES** ou **1 CORE**.
- **UNDO et COMPARE sur la FM-1** (EDIT → **UNDO**) : chaque changement du son devient une étape dès qu'il s'arrête (un balayage de knob = une étape), un son chargé aussi. KNOB 1 recule (gauche) et avance (droite) sur 16 étapes ; la page les liste. KNOB 2 **CMP** : ORIG fait entendre le son tel qu'il a été chargé, EDIT tes modifications. HOME affiche **EDITED** quand le son a été modifié, et charger un autre son ne perd jamais tes modifications : UNDO les ramène.
- **RANDOM par section** : WHAT propose aussi **ENV**, **MOD**, **FX** et **OPS** (sur FMX : les opérateurs eux-mêmes, comme le randomizer de l'opsix).
- **Favoris** : page PRESETS, KNOB 4 à droite met le son en haut de la liste (16 max), à gauche le retire.
- **De SLOOP 2.7** : **KBD HOLD** (ARP 2 → HOLD ON avec l'arpège OFF : les notes continuent de sonner, l'accord suivant les remplace), **15 modes d'arpège**, le **swing de l'arpège** marche aussi à l'arrêt, un son chargé pendant que l'arpège joue le garde, **CC 74** sur le cutoff propre à chaque moteur, **REC QUANT OFF** (KNOB 4 de l'écran REC), l'overdub sur des notes tenues, **l'accélération des knobs** (MENU → SCREEN → KNOB ACCEL), le **glissement de LEVEL et PAN**, les samples qui **bouclent tant que la touche est tenue**, **USB RATE** (MENU → AUDIO, pour Android), **CLOCK OUT** MIDI sur USB (MENU → MIDI).
- **L'éditeur web** : une carte **Sound tools** avec **Undo / Redo** (Ctrl+Z), **Compare with loaded**, **A / B**, un **randomizer par section** avec un dosage, la liste **de ce qui a changé** et **l'enveloppe dessinée**. La page Samples règle la **boucle** de chaque fichier (ONE SHOT, LOOP, LOOP+TAIL, ou la boucle du WAV lui-même). L'éditeur trouve la FM-1 même si Windows a gardé un autre nom de port.

## Nouveau dans la 0.6 : deux parties, un effet insert par son, FMX et le wave sequencing

- **MULTI (bi-timbral)** : EDIT → LAYER → MODE a un 4e choix, **MULTI** : le son du layer devient une partie à lui, sur son propre canal MIDI (**CH**, sur le bouton qui sert à SPLIT dans les autres modes ; 2 par défaut), avec son propre pitch bend, sa molette, sa pression et sa pédale de sustain ; tous les autres canaux, le clavier, le séquenceur et l'arpégiateur jouent le son principal. Les voix sont partagées (8, ou 12 / 16 avec `INSTALL-CLIPPER.bat 12` / `16`). LAYER et SPLIT comme avant.
- **Un effet INSERT par son** (FX → INSERT, INSERT 2), celui de SLOOP 2.6 : TYPE OFF, SOFT, HARD, FOLD, FUZZ (DRIVE TONE LEVEL), CRUSH (BITS RATE LPF), PHASR, FLANG, CHOR (RATE DEPTH FDBK), et MIX. Après la saturation du son, avant son filtre et les envois ; le son principal et le layer ont chacun le leur ; gardé avec le son (presets, presets utilisateur, projets). OFF ou MIX 0 : le son exactement comme avant. Un son ou un projet de SLOOP 2.6 garde son INSERT.
- **FMX**, un moteur à six opérateurs inspiré du Korg opsix : chaque opérateur a sa **forme d'onde** (SINE TRI SAW SQR HALF ABS QUART PULSE SIN8 NOISE) et son **mode** : **PM** (la « FM » du DX7), **FM** (la vraie, linéaire) ou **RING** ; un rapport (x0.5 à x31, FINE) ou une fréquence fixe ; niveau, ADSR, vélocité, suivi de clavier, désaccord. **Routage libre** : n'importe quel opérateur module n'importe quel autre, chaque liaison avec sa quantité, un opérateur sur lui-même c'est son feedback, n'importe lequel vers la sortie. 40 modèles (ALG : les 32 du DX7, puis CASCADE, 2 STACKS, STAR, 2 x 2>1, RING 3, LOOP, Y, 1 > 5) pour partir, puis chaque liaison se change. Une enveloppe de hauteur (PEG). EDIT 1 / 2 : PTCH ALG MOD FB / TIME DTN VEL KTRK (des macros sur le patch) ; sur FMX, six pages EDIT de plus modifient le patch lui-même : **OP** (OP WAVE MODE RATIO), **OP 2** (FINE LEVEL VEL KSC), **OP ENV**, **ROUTE** (TO FROM AMT OUT, avec le routage dessiné en grille), **FMX** (DTN FIX PEG PTIM) et **STORE** (dans la banque, B1..B27, la lecture arrêtée). La banque est celle de FM6 : un emplacement contient un patch FMX ou une voix DX7, et FMX joue une voix DX7 convertie. Dans l'éditeur : tout le patch, le routage dessiné, les modèles, LIVE, STORE, les fichiers (`.clipper-fmx.json`). 16 sons. Une voix coûte à peu près une voix de HYBRID.
- **WSEQ**, le « wave sequencing » inspiré du Korg Wavestation : chaque note joue une suite de pas, chacun avec sa forme d'onde, sa hauteur et son niveau, au tempo du morceau, le pas suivant arrivant en fondu sur la fin du précédent. SEQ (16 suites), RATE (1/32 à une mesure), XFADE, LOOP (FWD BACK PING ONCE), START, PITCH (l'écart des hauteurs des pas : 0 aucun, 127 le double), CUT, RES. 16 sons ; léger (un tiers de HYBRID).
- **360 sons, 21 moteurs.** L'éditeur web connaît les nouvelles pages (INSERT, et le patch de FMX).

## Nouveau dans la 0.5.1 : un relevé du temps audio (la chasse aux craquements)

GLO > SYSTEM, la colonne CPU alterne maintenant toutes les 1,5 s : **CPU** (ou l'état USB), **LATE** (les tampons audio calculés trop tard depuis l'allumage : chacun est un clic) et **PEAK** (le tampon le plus lent des dernières secondes, en % du temps dont il dispose ; au-dessus de 100, il est en retard). Si tu entends des craquements, regarde si LATE monte pendant qu'ils se produisent.

## Nouveau dans la 0.5 : deux moteurs de plus, VPS et TERRAIN

Tous deux proposés dans les discussions de Felucca (#145, #146) comme des façons légères d'obtenir des sons qu'aucun oscillateur ne fait ; chacun coûte à peu près ce que coûte ANALOG (un tiers de HYBRID).

- **VPS**, la synthèse par « vector phaseshaping » (d'après Kleimola, Lazzarini, Timoney et Välimäki, DAFx 2011) : un sinus lu à travers une phase pliée. Le pli est un point : **D** (où dans la période) et **V** (combien de cycles à ce moment-là). V sous 1 : des ondes à distorsion de phase, façon CZ ; V au-dessus de 1 : V cycles serrés dans le début de la période, un formant à V fois la note, comme un hard sync ou un balayage de voyelle. Uniquement des cycles entiers (entre deux entiers, V fond les deux) : pas de pointe, pas de repliement. **VENV** balaie V avec l'enveloppe à chaque note, **OSC2** ajoute un 2e oscillateur au D inversé et désaccordé (**DTN**), **SUB** un sinus à l'octave du dessous, puis CUT et RES. La modulation SHAPE (LFO, enveloppe, molette, matrice) déplace D. EDIT : D V VENV OSC2 / CUT RES DTN SUB. 10 sons.
- **TERRAIN**, la synthèse par « wave terrain », version légère : une orbite tourne au-dessus d'une surface, et la hauteur de la surface sous elle est le son. Un tour est une période : le son a une hauteur juste ; la position de l'orbite (**X**, **Y**) et sa largeur (**RAD**) choisissent les harmoniques ; **ORB** l'aplatit ou en fait un huit ; **DRIFT** la déplace lentement, chaque note à sa façon, pour des timbres qui évoluent sans cesse. Huit surfaces de 32 × 32 (**TERR** : WAVES, RIPPLE, CRYSTAL, BUMPS, SADDLE, FOLDS, TWIST, CHAOS), périodiques dans les deux sens et à bande limitée ; l'orbite se resserre dans les aigus. EDIT : TERR RAD X Y / ORB DRIFT CUT RES. 10 sons.
- **328 sons, 19 moteurs.** Chaque réglage EDIT des deux est une destination de la matrice (X et Y bougés par LFO 2 ou par la position de la touche, par exemple).

## Nouveau dans la 0.4.1 : les correctifs de SLOOP 2.6, et la cohabitation avec SLOOP 2.6

- **Une sortie plus propre** : le limiteur du master regarde un bloc (0,7 ms) en avance : une attaque forte est déjà baissée quand elle arrive, au lieu que ses premiers échantillons passent dans l'écrêteur doux de la sortie (un craquement avec MASTER fort, surtout au casque) ; une basse forte tenue garde un seul gain (plus de sifflement). D'après SLOOP 2.6.
- **Plus de clic quand un filtre commence à se fermer** : le FILTER du son et le FILT du master démarrent leur passe-bas là où est le signal (il partait du silence sous le son : un craquement).
- **FM6** : un son changé pendant que des notes tiennent les fait disparaître avec leur propre patch ; le nouveau patch vient après (elles le prenaient tout de suite : un clic).
- **Arpégiateur** (#191, d'après Felucca 1.5) : un arpège verrouillé suit TRN, ROOT, SCALE et QNT changés après l'appui sur ses touches.
- **Charger un projet ou NEW** remet le pitch bend au milieu, la pression à 0 et relève la pédale de sustain (ses notes s'arrêtent) ; les bends MPE de chaque note aussi.
- **Éditeur web** : une valeur remise (la même sur un autre son, ou après qu'un knob l'a bougée sur la FM-1) est renvoyée (elle était ignorée pour de bon) ; un fichier de son d'une autre version se charge selon sa disposition ; une sauvegarde avec des projets plus récents que le firmware s'arrête avant d'écrire quoi que ce soit.
- **Installateur** : il trouve une FM-1 qui tourne sous SLOOP (passer de SLOOP à CLIPPER ne s'arrête plus sur *FM-1 introuvable*) ; sinon, le message liste les ports MIDI vus.
- **Passer de SLOOP 2.6 à CLIPPER sur la même FM-1** : CLIPPER lit les projets de SLOOP 2.6 (ils apparaissaient comme des emplacements vides, et sauvegarder dessus les écrasait) ; il garde les réglages MIDI et le MIDI LEARN de SLOOP 2.6, et les kits SYN de SLOOP ; son réglage MPE n'est plus rangé là où SLOOP 2.6 garde GLO > SYSTEM > IN. Les presets utilisateur redeviennent une seule banque pour les deux (enregistrements de 192 octets : la première version 0.4 les passait à 232, que SLOOP lisait comme vides ; ils sont convertis à l'allumage) : la matrice, LFO 2 et ENV 2 sont rangés dans les valeurs libres de l'enregistrement, SLOOP voit le son tel quel. Un projet garde LFO 2, ENV 2 et la matrice dans les emplacements de lock libres de la piste 3, que SLOOP ignore (la première 0.4 les mettait dans les réglages des pistes 3 et 4 ; toujours lus).

## Nouveau dans la 0.4 : une matrice de modulation sur tous les moteurs, et le MPE

- **MATRIX** (LFO → MATRIX), sur tous les moteurs : six slots, chacun une **SOURCE** → une **DESTINATION** × une **QUANTITÉ** (±). SLOT choisit le slot que les trois autres knobs règlent ; l'écran montre les six. Sources : **LFO1**, **LFO2**, **ENV1** (l'enveloppe de volume), **ENV2**, **VEL**, **KEY** (la note, autour du Do 4), **MW** (molette), **AT** (aftertouch, canal ou polyphonique), **RAND** (une valeur au hasard à chaque note), **SLIDE** (MPE). Destinations : **PITCH** (jusqu'à ±24 demi-tons, plus fin près de 0), **CUT**, **SHAPE**, **AMP**, **PAN**, **LFO2** (sa vitesse) et n'importe lequel des huit réglages EDIT du moteur, par son nom (CUT d'ANALOG, POS de WAVE, les ratios de FM6…), note par note. Un slot à 0 ne coûte rien et ne change rien.
- **LFO 2** (LFO → LFO 2) : RATE, WAVE (SIN TRI SAW SQR S&H), MODE : **FREE** (un pour le son) ou **KEY** (un par note, relancé à chaque note).
- **ENV 2** (ENV → ENV 2) : une 2e enveloppe ADSR, pour la matrice.
- **MPE** (HOME maintenu → SYSTEM → MPE) : OFF, ou la plage de bend des canaux de notes (48 ST, le standard MPE, ou 2, 12, 24, 96). Pour l'Osmose d'Expressive E, les ROLI, la LinnStrument, etc. : le canal 1 est le canal maître (bend, molette, pédale de toutes les notes), les canaux 2 à 16 portent chacun une note avec **son propre pitch bend**, **sa propre pression** (la source AT et MOD AT>) et **son propre slide** (CC 74 : la source SLIDE ; si aucun slot ne l'utilise, il ouvre le filtre de la note).
- **Gardés avec le son** : les projets gardent LFO 2, ENV 2 et la matrice des deux sons (le layer aussi) dans une place que le synthé n'utilisait pas (0.4.1 : les emplacements de lock libres de la piste 3) ; un projet garde sa taille, les anciens se chargent avec la matrice vide. Les presets utilisateur les gardent aussi (0.4.1 : dans l'enregistrement que SLOOP lit). RANDOM ALL bouge aussi LFO 2, ENV 2 et les quantités des slots utilisés ; MORPH les fait tous glisser.

## Nouveau dans la 0.3 : jouer comme sur un grand synthé

- **EQ et compresseur du master** : FX → **EQ+COMP** : LOW (étagère à 150 Hz), MID (cloche à 1 kHz), HIGH (étagère à 5 kHz), ±12 dB (MID ±10) par pas de 1 dB, et **COMP**, un compresseur de bus stéréo (le « glue » : un seul bouton baisse le seuil de -6 à -30 dB et monte le ratio de 2:1 à 4:1, avec gain de compensation automatique). À plat et à 0, ils sont court-circuités. Tout glisse, aucun clic.
- **Layer et split** : EDIT → **LAYER** : MODE (OFF / LAYER / SPLIT), SOUND (n'importe lequel des 308 sons ou de tes presets comme 2e son), SPLIT (la note de partage : le layer en dessous, le son principal à partir d'elle), LEVEL ; **LAYER 2** : OCT (le layer de -2 à +2 octaves), PAN, FILT et **SWAP** (les deux sons échangent leur place : toutes les pages éditent alors l'autre son ; SWAP encore pour revenir). Clavier, MIDI, séquenceur et arpégiateur jouent les deux.
- **Unisson polyphonique sur tous les moteurs** : EDIT → VOICE : **UNI2** et **UNI4** empilent 2 ou 4 voix désaccordées par note (DTUNE sur VOICE 2). UNI reste l'empilement mono de 8 voix.
- **Morph et randomiseur** : EDIT → **MORPH** : SET A, SET B, puis MORPH fait passer tout le son de l'un à l'autre (MIDI CC 20). EDIT → **RANDOM** : AMT, WHAT (EDIT, +ENV, ALL), RAND, et BACK pour annuler.
- **Modulation** : LFO → **MOD** : VEL>F (vélocité vers le cutoff), MW> et AT> (molette et aftertouch) vers VIB (vibrato 5,5 Hz), CUT, SHP, WAH ou TREM. Sur tous les moteurs (HYBRID garde sa matrice).
- **Changements de son sans clic** : un preset, un moteur, un preset utilisateur, SWAP, RAND ou BACK changé pendant que des notes sonnent les fait disparaître en 6 ms avec leurs propres réglages ; les notes jouées pendant ce temps attendent le nouveau son.
- **WAVE charge 6 fois plus vite** : les tables sont calculées par FFT inverse au lieu de sommes additives, 4 frames par passage de la boucle : une nouvelle table est prête en 4 passages.
- **Plus de visualiseur plein écran** (son code et 5,5 Ko de mémoire vont au synthé) ; HOME garde son oscilloscope.
- **308 sons** : 99 nouveaux sur tous les moteurs, chacun au même niveau à travers la vraie chaîne de sortie du synthé.

## Nouveau dans la 0.2 : le son

- **Ladder MOOG** : un filtre 4 pôles 24 dB à retour sans délai, avec une saturation tanh à l'entrée. Il chauffe quand on le pousse, sa résonance sature en douceur et il auto-oscille juste. C'est un type de filtre dans HYBRID. Sur ANALOG : FX → FILTER → **MODL** (SVF 12 dB ou MOOG 24 dB).
- **Résonance douce** : les filtres SVF d'HYBRID saturent en douceur au lieu d'écrêter.
- **Anti-aliasing** : triangles à bande limitée, hard sync corrigé à chaque reset, FM, feedback et wavefolder tenus sous 20 kHz dans les aigus.
- **Réverb plate** (d'après Dattorro) : FX → REVERB : TYPE (ROOM ou PLATE), SIZE, DAMP et PRE (pré-délai en ms). Le chorus a sa propre page CHORUS.
- **WAVE, tables d'ondes** : 16 tables de 16 frames (formes classiques, PWM, sync, FM, fold, formants, balayages façon PPG, orgue, chip, comb…). POS les parcourt, et l'enveloppe et le LFO le font bouger (ENV DEST et LFO DEST → SHP). DTN ajoute un 2e oscillateur, SUB un sinus à l'octave du dessous, puis le filtre d'ANALOG (SVF ou MOOG). Sans repliement. Tes propres tables (WAV façon Serum) s'envoient dans un emplacement USR depuis l'onglet Samples de l'éditeur, mode WAVETABLE ; TABL = USR1 à USR4 les joue.
- **ACID** : une basse façon 303, une voix. Dent de scie ou carré dans le ladder, puis DIST. L'enveloppe de filtre (ENV, DEC), l'**accent** (une note à vélocité 120 ou plus, les pas ACCENT du séquenceur : plus fort, enveloppe plus courte et plus profonde) et son **balayage** (SWP : une suite d'accents pousse le cutoff de plus en plus haut, le « squelch »), et les **slides** : un pas SLIDE, ou du legato avec GLIDE en mode TIME, glisse sans relancer l'enveloppe. EDIT : WAVE CUT RES ENV / DEC ACC DIST SWP.
- **STRINGS** : une string machine des années 70 (Solina, ARP, Elka). Registres 16', 8' et 4', de la dent de scie au carré (SHP), un ensemble à trois lignes (balayage lent et vibrato rapide : ENS, RATE), un passe-bas qui suit le clavier. EDIT : 16' 8' 4' ENS / CUT RATE SHP RES.
- **MACRO** : quatre modèles inspirés de Mutable Instruments Plaits sur trois boutons HARM, TIMB, MORPH : **CHORD** (onze accords, renversements), **SWARM** (huit oscillateurs qui dérivent), **ADDTV** (16 harmoniques sous une bosse mobile), **SPEECH** (des formants qui disent AI, AU, OI, UA, WOW, YEAH, AEIOU, MAMA, tenus ou parlés). Puis un passe-bas et un **low-pass gate** (DEC, LPG). EDIT : MODL HARM TIMB MRPH / CUT RES DEC LPG.
- **Plus de samples intégrés** : 300 Ko de flash libérés pour la synthèse. SAMPLE et GRAIN jouent tes propres samples (onglet Samples de l'éditeur, 4 emplacements). 209 sons d'usine en 0.2 (308 en 0.3) : cinq nouveaux sur le ladder (MOOG BASS, MOOG LEAD, LADDER PAD, MINI BASS, MINI LEAD), seize sur WAVE, huit sur ACID, huit sur STRINGS, douze sur MACRO. 17 moteurs.

## 2. Sur la FM-1

| | |
| --- | --- |
| **HOME** | L'écran du synthé : le nom du son, sa famille, son numéro dans la liste, le moteur et un oscilloscope. Les 4 knobs règlent les contrôles principaux du son. |
| **PRESETS** | Tous les sons, un par un. |
| **ALGORITHM** | Saute à la famille suivante (PAD, BASS, LEAD, PLCK, KEYS, STAB, FX…). |
| **FX, EDIT, ARP, GLO, SAVE** | Un appui ouvre les pages. Bouton maintenu, le clavier joue toujours. Pages FX : FX, FILTER (avec MODL), DLY, REVERB, CHORUS. |
| **SEQ, SEL** | Maintenus : SEQ montre les 16 pas sur les touches, SEL la tonalité et les accords. Un appui ouvre leurs pages. |
| **REC** | Enregistre ce que tu joues dans le pattern. REC maintenu efface le pattern. |
| **MIDI** | Omni : tous les canaux jouent le synthé. |

Un projet de SLOOP se charge avec seulement sa première piste.

## 3. Jouer HYBRID

- Tourne **PRESETS** (ou **ALGORITHM** pour changer de famille). Les sons HYBRID sont rangés dans les familles : **CLIPPER**, POLY PAD, STRINGS, VOX, GLASS PAD et DRONE (PAD) ; ACID, WOBBLE, REESE, FOLD BASS et 808 SUB (BASS) ; SYNC LEAD, HOOVER, CHIP LEAD et TRANCE (LEAD) ; PLUCK, SAW PLUCK, FM BELL, SPACE BEL et METAL (PLCK) ; FM KEYS (KEYS) ; BRASS (STAB) ; RISER et INIT (FX).
- Sur **HOME**, les 4 knobs règlent **CUT**, **RES**, **M1** et **M2**.
- Page **EDIT** : CUT, RES, FENV (décalages sur les deux filtres), **PTCH** (le patch d'usine), et **M1 à M4**, quatre macros dont chaque patch choisit l'effet. Par exemple, sur CLIPPER, M1 ouvre le filtre et M2 élargit les supersaws.
- Avec un clavier MIDI : pitch bend, molette (vibrato sur plusieurs patchs), aftertouch, sustain, et M1 à M4 sur les CC 16 à 19.

## 4. Éditer tout le patch dans l'éditeur

Double-clique sur `OPEN-EDITOR.bat` (http://localhost:8767/webapp/editor/), clique sur **Connect**, puis choisis un son HYBRID. Les pages Tracks, Song et Drum kit n'apparaissent plus avec Clipper. Dans l'onglet **Sound**, le panneau **HYBRID patch** montre tout : oscillateurs, filtres et routage, ENV2 et ENV3, LFO2 et LFO3 (synchro au tempo possible), la matrice de 16 slots et le bend. Avec **live** coché, chaque réglage s'entend tout de suite.

Les autres boutons du panneau :
- **Export patch** et **Import patch** gardent tes sons dans des fichiers `.clipper.json`.
- **factory** + **load** recharge un patch d'usine.

Un projet garde le PTCH, mais pas le patch modifié : exporte-le pour le garder.

Pour essayer l'éditeur sans la FM-1, ouvre `web/editor.html?mock=1` dans Chrome.

## 5. Le test dont j'ai besoin : les deux cœurs et la charge processeur

C'est ce qui décide combien de voix Clipper peut jouer. La FM-1 doit être branchée en USB à l'ordinateur, sinon la case CPU affiche l'état USB à la place.

1. Après l'installation, tiens HOME (le menu) → **SYS** → **ABOUT** : il doit afficher **2 CORES**. Si c'est **1 CORE**, dis-le-moi.
2. Choisis le son **CLIPPER** avec PRESETS, va sur **GLO → SYSTEM** : la 4e case alterne **CPU**, **LATE** et **PEAK**.
3. Tiens **8 notes** et note CPU et PEAK. Puis MENU → SYS → **2ND CORE OFF**, et refais la même mesure : la différence, c'est le gain du 2e cœur. Remets **ON**.
4. Refais avec **POLY PAD**, **TRANCE**, un son **FMX** (FMX EP) et **ENSEMBLE** (STRINGS).
5. Si PEAK reste sous ~70 % et LATE ne monte pas, ferme la fenêtre, puis lance **`INSTALL-CLIPPER.bat 12`** (12 voix) et refais la mesure avec 12 notes. Si ça tient encore, essaie **`INSTALL-CLIPPER.bat 16`**.

Envoie-moi les chiffres (avec et sans 2ND CORE). Signale aussi un son qui craque, une note qui reste coincée, un écran **FELUCCA CRASH** (prends-le en photo : il reste 10 s) ou un écran bizarre.

## Revenir en arrière

- **Vers SLOOP** : lance `INSTALL-SLOOP.bat` depuis le dossier de SLOOP. Un son réglé sur HYBRID y jouera un autre moteur.
- **Vers le firmware officiel** : passe par la page d'installation (« back to the official V15 »), après avoir fait une sauvegarde.
