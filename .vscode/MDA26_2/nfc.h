#ifndef NFC_H
#define NFC_H

// Prototipos de funciones
bool init_nfc();
String read_nfc_uid();
bool is_nfc_authorized(String uid);
bool add_nfc_authorized(String uid);
bool remove_nfc_authorized(String uid);

#endif