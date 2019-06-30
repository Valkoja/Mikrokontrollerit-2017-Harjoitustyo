#include <Ethernet.h>
#include <SPI.h>
#include <LiquidCrystal.h>
#include <TimerOne.h>

enum State {
    CLOCK,
    PASSWORD,
    CONNECTED,
    DISCONNECTED,
    UNLOCKED,
    DENIED
};

// LCD ja ethernet olioiden luonti
LiquidCrystal lcd(8, 7, 6, 5, 4, 3);
EthernetClient client;

// Globalit muuttujat
int clearCount = 0;
State stateMachine;
String userInput;

void setup()
{
    // Verkon alustus
    byte deviceIP[]  = {10, 0, 0, 11};
    byte deviceMac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
    Ethernet.begin(deviceMac, deviceIP);

    // Sarjaportin, ajastimen ja nayton alustukset
    Serial.begin(9600);
    Timer1.initialize();
    lcd.begin(16, 2);

    // Nappaimiston alustus -> portit A0-A3 outputeiksi
    DDRC  = B00001111;
    PORTC = B11110000;


    // Avausnappi inputiksi, avausvalo outputiksi
    pinMode(2, INPUT_PULLUP);
    pinMode(9, OUTPUT);

    // INT0 p��lle, laukeaa nousevalla reunalla
    EIMSK = 0x01;
    EICRA = 0x02;

    // Siirrytaan kellomoodiin
    switchToClock();
}

void loop()
{
    readKeyboard();
}

// Nappaimiston lukufunktio
void readKeyboard()
{
    int pinValue;

    for (int i = 0; i < 4; i++)
    {
        // Asetetaan vuorollaan jokin output pinneista maihin
        switch (i)
        {
            case 0:
                PORTC = B00001110;
                break;
            case 1:
                PORTC = B00001101;
                break;
            case 2:
                PORTC = B00001011;
                break;
            case 3:
                PORTC = B00000111;
                break;
        }

        // Antaa analogiporteille hetken aikaa asettua
        delay(10);

        for (int j = 0; j < 4; j++)
        {
            // Alustetaan lahtotilanne ylos, varmuuden vuoksi
            pinValue = 1023;

            // Luetaan jokaisen pinnin (napin) tila vuorollaan
            switch (j)
            {
                case 0:
                    pinValue = analogRead(4);
                    break;
                case 1:
                    pinValue = analogRead(5);
                    break;
                case 2:
                    pinValue = analogRead(6);
                    break;
                case 3:
                    pinValue = analogRead(7);
                    break;
            }

            // Dokumentaation mukaan lukemisessa voi menna hetki
            delay(10);

            // Oliko luettu nappi pohjassa
            if (pinValue < 1)
            {
                // Odotetaan hetki ja tarkistetaan onko nappi yha pohjassa
                delay(100);
                pinValue = analogRead(j + 4);

                if (pinValue < 1)
                {
                    // Kutsutaan funktiota kasittelemaan input
                    switch (stateMachine)
                    {
                        case CLOCK:
                            switchToInput(i * 4 + j);
                            break;
                        case PASSWORD:
                            parseKeyboard(i * 4 + j);
                            break;
                    }

                    // Odotetaan kunnes nappain ei ole enaan pohjassa (yritetaan estaa tuplapainallukset)
                    while (pinValue < 1)
                    {
                        pinValue = analogRead(j + 4);
                        delay(20);
                    }
                }
            }
        }
    }
}

// A, B, C, * ei tee mit��n, D = delete input->tyhjent�� salasanan, # = submit
void parseKeyboard(int keyCode)
{
    switch (keyCode)
    {
        case 0: // 1
            userInput += "1";
            break;

        case 1: // 2
            userInput += "2";
            break;

        case 2: // 3
            userInput += "3";
            break;

        case 3: // A
            break;

        case 4: // 4
            userInput += "4";
            break;

        case 5: // 5
            userInput += "5";
            break;

        case 6: // 6
            userInput += "6";
            break;

        case 7: // B
            break;

        case 8: // 7
            userInput += "7";
            break;

        case 9: // 8
            userInput += "8";
            break;

        case 10: // 9
            userInput += "9";
            break;

        case 11: // C
            break;

        case 12: // *
        switchToClock();
            break;

        case 13: // 0
            userInput += "0";
            break;

        case 14: // #
            requestUnlock();
            break;

        case 15: // D
            userInput = "";
            break;
    }

    // Jos ollaa yha syottotilassa, tulostetaan ruudulle
    if (stateMachine == PASSWORD)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("PIN Code: ");

        // Onko kayttaja syottanyt jo jotain
        if (userInput.length() > 0)
        {
            lcd.setCursor(0, 1);

            // Tulostetaan aikaisemmat merkit tahtina
            if (userInput.length() > 1)
            {
                for (int i = 0; i < userInput.length() - 1; i++)
                {
                    lcd.print("*");
                }
            }

            // Tulostetaan viimeinen merkki selkokielisena
            lcd.print(userInput.substring(userInput.length() - 1));
        }
    }
}

// Sis��nrakennettu interrupt funktio
ISR (INT0_vect)
{
    noInterrupts();
    delayMicroseconds(5000);
    if(digitalRead(2) == LOW)
    {
    requestOverride();
    }
    interrupts();
}

void requestTime()
{
    // Virhetilanteiden varalta tyhjennetaan ruutu joka 10. kierros
    if (clearCount++ >= 10)
    {
        lcd.clear();
        clearCount = 0;
    }

    // Pyydetaan palvelimelta kellonaika
    String httpStr = sendRequest("GET /rest.php?action=clock HTTP/1.0");

    // Kaivetaan saadusta syotteesta halutut osat ja tulostetaan ne n�yt�lle
    if (httpStr.length() > 18)
    {
        String timeStr = httpStr.substring(httpStr.length() - 18, httpStr.length() - 10);
        String dateStr = httpStr.substring(httpStr.length() - 10);

        lcd.setCursor(0, 0);
        lcd.print(timeStr);

        lcd.setCursor(0, 1);
        lcd.print(dateStr);
    }
}

void requestUnlock()
{
    // Pyydetaan palvelimelta tieto onko avaimela oikeus oveen
    String httpStr = sendRequest("GET /rest.php?action=unlock&key=" + userInput + " HTTP/1.0");

    // Onko vastauksen viimeinen merkki 1 vai 0
    if (httpStr.length() > 0)
    {
        if (httpStr.endsWith("1"))
        {
            switchToUnlocked();
        }
        else
        {
            switchToDenied();
        }
    }
    else
    {
        // Palvelin vastasi tyhjan stringin, ei pitaisi tapahtua
        switchToClock();
    }
}

void requestOverride()
{
    // Logitetaan oven avaaminen napilla
    String httpStr = sendRequest("GET /rest.php?action=override HTTP/1.0");

    // Avataan lukko
    switchToUnlocked();
}

// Funktio palvelimelle l?aetettaville requesteille
String sendRequest(String request)
{
    int timeout    = 0;
    int retries    = 0;
    char readChar  = ' ';
    String result  = "";
    byte server[]  = {10, 0, 0, 10};
    State oldState = stateMachine;

    // Yhteys palvelimeen
    while (!client.connected())
    {
        if (client.connect(server, 80))
        {
            // No commection -ilmoitus pois ja tilakone takaisin
            if (stateMachine == DISCONNECTED)
            {
                lcd.clear();
                stateMachine = oldState;
            }

            // Lahetetaan
            Serial.println(request);
            client.println(request);
            client.println();
        }
        else
        {
            stateMachine = DISCONNECTED;

            lcd.setCursor(0, 0);
            lcd.print("No Connection");

            // LCD naytto paivittyy joka kerta kun yhteys epaonnistuu -> nakee ettei ohjelma ole jumissa
            lcd.setCursor(0, 1);

            if (retries++ % 2 == 0)
            {
                lcd.print("Retrying...");
            }
            else
            {
                lcd.print("Retrying.. ");
            }

            delay(500);
        }
    }

    // Luetaan GET requestin vastaus
    while (timeout < 100)
    {
        if (client.available())
        {
            readChar = client.read();
            result += readChar;
        }
        else if (result.length() == 0)
        {
            delay(5);
            timeout++;
        }
        else
        {
            timeout++;
        }
    }

    // Suljetaan yhteys, palautetaan palvelimelta saatu vastaus
    client.stop();
    return result;
}

// Vaihdetaan kellotilaan
void switchToClock()
{
    // Irroitetaan vanha interrupt, vaihdetaan tilaa
    Timer1.detachInterrupt();
    stateMachine = CLOCK;

    // Tyhjennetaan naytto, haetaan ensimmainen kellonaika valittomasti
    lcd.clear();
    requestTime();

    // Asetetaan kellon hakeminen sekunnin v�lein
    Timer1.setPeriod(1000000);
    Timer1.attachInterrupt(requestTime);
}

void switchToInput(int keyCode)
{
    // Irroitetaan vanha interrupt, vaihdetaan tilaa
    Timer1.detachInterrupt();
    stateMachine = PASSWORD;

    // Nollataan aikaisempi yritys
    userInput = "";

    // Lahetetaan nappainkoodi tulkattavaksi
    parseKeyboard(keyCode);
}

void switchToUnlocked()
{
    // Irroitetaan vanha interrupt, vaihdetaan tilaa
    Timer1.detachInterrupt();
    stateMachine = UNLOCKED;

    // Tyhjennetaan naytto, ilmoitus oven tilasta
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Door is open");

    // Valo paalle, odotetaan 5 sekuntia, valo pois, siirrytaan kelloon
    digitalWrite(9, HIGH);
    for (unsigned int i = 0; i < 5000;i++)
    {
        delayMicroseconds(1000);
    }
    digitalWrite(9, LOW);
    switchToClock();
}

void switchToDenied()
{
    // Irroitetaan vanha interrupt, vaihdetaan tilaa
    Timer1.detachInterrupt();
    stateMachine = DENIED;

    // Tyhjennetaan naytto, ilmoitus vaarasta avaimesta
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Wrong PIN");

    // Odotetaan 5 sekuntia, siirrytaan kelloon
    delay(5000);
    switchToClock();
}
