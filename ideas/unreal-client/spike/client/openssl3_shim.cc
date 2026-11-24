// THROWAWAY — link shim for the spike only.
//
// TurboLink's Linux libs were built against Unreal's OpenSSL 1.1.1, where
// SSL_get_peer_certificate exists (deprecated). OpenSSL 3 removed it in
// favour of SSL_get1_peer_certificate; this container has OpenSSL 3. Inside
// Unreal the engine's 1.1.1 provides the symbol and this file is not needed.
#include <openssl/ssl.h>
#include <openssl/x509.h>

// OpenSSL 3's ssl.h aliases the old name as a macro; the reference inside
// libgrpc.a is to the real symbol, so define it under its own name.
#undef SSL_get_peer_certificate

extern "C" X509* SSL_get_peer_certificate(const SSL* ssl) {
  return SSL_get1_peer_certificate(ssl);
}
