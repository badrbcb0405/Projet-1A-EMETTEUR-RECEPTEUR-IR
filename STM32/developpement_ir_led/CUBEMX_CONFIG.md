# Configuration CubeMX detaillee

Cette fiche donne une configuration concrete pour le STM32L476RG. Tu peux changer les pins,
mais garde la meme logique.

## Emetteur IR

### Timer PWM IR

Nom conseille : `TIM1_CH1`

- Mode : PWM Generation CH1
- Frequence : 38 kHz environ
- Duty cycle : 33 %
- Sortie vers transistor de LED IR

Exemple avec timer a 80 MHz :

| Parametre | Valeur |
|---|---:|
| Prescaler | `0` |
| Counter Period | `2104` |
| Pulse | `702` |

La frequence obtenue est :

```text
80 MHz / (2104 + 1) = 38005 Hz
```

### Boutons

Entrees GPIO avec pull-up interne :

| Fonction | Nom CubeMX conseille |
|---|---|
| Marche/arret | `BUTTON1` |
| Luminosite + | `BUTTON2` |
| Luminosite - | `BUTTON3` |
| Mode | `BUTTON4` |

Chaque bouton doit relier la broche a GND quand on appuie.

## Recepteur IR et LED WS2812B

### Entree IR

Nom conseille : `IR_IN`

- GPIO input avec interruption externe.
- Trigger : Rising/Falling edge.
- Pull-up : generalement active, sauf si le module possede deja une sortie fortement tiree au haut.

Branchement type module VS1838B/TSOP :

```text
VCC  -> 3.3 V ou 5 V selon module
GND  -> GND commun
OUT  -> IR_IN STM32
```

Attention : si le module est alimente en 5 V, verifie que sa sortie est compatible 3.3 V avec ton STM32.

### Timer base de temps IR

Nom conseille : `TIM2`

- Mode : Time Base
- Frequence compteur : 1 MHz
- Interruption timer non necessaire

Exemple avec timer a 80 MHz :

| Parametre | Valeur |
|---|---:|
| Prescaler | `79` |
| Counter Period | `4294967295` |

### Ligne DATA des LED

Nom conseille : `LED_DATA`

- Mode : GPIO Output Push-Pull
- Pull : No pull
- Vitesse : Very High
- Sortie vers l'entree `DIN` de la premiere LED WS2812B

Les WS2812B sont pilotees par une trame numerique sur `LED_DATA`; les LED suivantes
recoivent le signal via `DOUT`.

Branchement logique :

```text
STM32 LED_DATA -> adaptation logique -> DIN WS2812B
5 V LED        -> VDD WS2812B
GND LED        -> GND commun
```

## Ou copier les fichiers

Dans chaque projet CubeIDE :

```text
Core/Inc/ir_nec.h
Core/Src/ir_nec.c
```

Emetteur :

```text
Core/Inc/app_ir_remote.h
Core/Src/app_ir_remote.c
```

Recepteur :

```text
Core/Inc/app_ir_receiver.h
Core/Src/app_ir_receiver.c
Core/Inc/app_ws2812.h
Core/Src/app_ws2812.c
```

Puis adapte ton `main.c` a partir des exemples fournis.
