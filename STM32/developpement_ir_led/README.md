# Telecommande IR + recepteur LED WS2812B sur STM32


- un emetteur IR NEC 38 kHz ;
- un recepteur IR NEC ;
- une commande de LED RGB adressables WS2812B par une broche DATA.


## Hypotheses materielles

### Emetteur

- STM32 avec HAL.
- LED IR pilotee par une sortie PWM a 38 kHz.
- 4 boutons : `BUTTON1`, `BUTTON2`, `BUTTON3`, `BUTTON4`.
- Fonctions associees : marche/arret, luminosite +, luminosite -, mode.
- Boutons cables entre l'entree GPIO et GND, avec pull-up interne activee.

### Recepteur

- Module recepteur IR demodule 38 kHz, type VS1838B, TSOP1838, KY-022.
- Sortie du module reliee a une entree EXTI configuree sur fronts montant et descendant.
- 5 LED adressables WS2812B alimentees en 5 V.
- Ligne `LED_DATA` reliee a l'entree `DIN` de la chaine WS2812B.
- Adaptation logique 3.3 V vers 5 V si necessaire selon le shield.
- Masse commune entre STM32, alimentation LED et module IR.

## Configuration CubeMX conseillee

### Emetteur

1. Activer un timer PWM pour la LED IR.
2. Regler la frequence PWM a environ 38 kHz.
3. Duty cycle conseille : 33 %.
4. Activer les GPIO boutons en `GPIO_Input`, pull-up.
5. Ajouter les fichiers :
   - `common/ir_nec.h`
   - `common/ir_nec.c`
   - `emitter/app_ir_remote.h`
   - `emitter/app_ir_remote.c`

### Recepteur

1. Activer un timer libre a 1 MHz pour mesurer les durees IR en microsecondes.
2. Configurer l'entree IR en EXTI sur fronts montant et descendant.
3. Configurer la broche `LED_DATA` en `GPIO_Output`, vitesse tres haute.
4. Ajouter les fichiers :
   - `common/ir_nec.h`
   - `common/ir_nec.c`
   - `receiver/app_ir_receiver.h`
   - `receiver/app_ir_receiver.c`
   - `receiver/app_ws2812.h`
   - `receiver/app_ws2812.c`
5. Copier le contenu utile de `receiver/main_receiver_example.c` dans ton `Core/Src/main.c`.

## Codes IR utilises

Adresse NEC : `0x10EF`

| Bouton | Commande |
|---|---:|
| POWER | `0x45` |
| PLUS | `0x46` |
| MINUS | `0x47` |
| MODE | `0x44` |



## Notes importantes

- Le protocole NEC transmet les octets LSB first. Le code gere l'encodage et le decodage.
- La plupart des modules IR demodules sortent un signal actif a l'etat bas pendant les impulsions IR.
