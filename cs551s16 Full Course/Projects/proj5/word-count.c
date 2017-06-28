#include "common.h"

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int
make_client_socket(const char *host, const char *port)
{
  struct addrinfo hints = {
    .ai_flags = 0,
    .ai_family = AF_INET,
    .ai_socktype = SOCK_STREAM,
    .ai_protocol = 0
  };
  struct addrinfo *addr_list;
  //could use gethostbyaddr(), but getaddrinfo() preferred for new code
  int err = getaddrinfo(host, port, &hints, &addr_list);
  if (err < 0) {
    fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(err));
    exit(1);
  }
  int s = -1;
  for (const struct addrinfo *ap = addr_list; ap != NULL; ap = ap->ai_next) {
    s = socket(ap->ai_family, ap->ai_socktype, ap->ai_protocol);
    if (s < 0) continue;
    if (connect(s, ap->ai_addr, ap->ai_addrlen) != -1) break;
    close(s); s = -1;
  }
  if (s < 0) {
    fprintf(stderr, "could not connect to %s:%s\n", host, port);
    exit(1);
  }
  freeaddrinfo(addr_list);
  return s;
}

static void
send_request(int nArgs, const char *args[], int socket)
{
  int s = dup(socket);
  if (s < 0) {
    fprintf(stderr, "cannot dup socket: %s\n", strerror(errno));
    exit(1);
  }
  FILE *out = fdopen(s, "w");
  if (out == NULL) {
    fprintf(stderr, "cannot create requests file: %s\n", strerror(errno));
    exit(1);
  }
  fprintf(out, "%d%c", nArgs, '\0');
  for (int i = 0; i < nArgs; i++) {
    fprintf(out, "%s%c", args[i], '\0');
  }
  if (fclose(out) != 0) {
    fprintf(stderr, "cannot close requests file: %s\n", strerror(errno));
    exit(1);
  }
  if (shutdown(socket, SHUT_WR) < 0) {
    fprintf(stderr, "cannot shut down socket for writing: %s\n",
            strerror(errno));
    exit(1);
  }
}

/** Copy fd0 to out */
static void
out_response(int fd0, FILE *out)
{
  FILE *in = fdopen(fd0, "r");
  if (!in) {
    fprintf(stderr, "unable to open input FILE: %s\n", strerror(errno));
    exit(1);
  }
  for (int c = fgetc(in); c != EOF; c = fgetc(in)) {
    fputc(c, out);
  }
  fflush(out);
  if (fclose(in) != 0) {
    fprintf(stderr, "cannot close input file: %s\n", strerror(errno));
    exit(1);
  }
}

int
main(int argc, const char *argv[])
{
  if (argc < 6) {
    fprintf(stderr,
            "usage: %s HOSTNAME PORT N STOP_WORDS FILE1...\n",
            argv[0]);
    exit(1);
  }
  int port __attribute__((unused)) = get_port(argv[2]);
  int socket = make_client_socket(argv[1], argv[2]);
  send_request(argc - 3, &argv[3], socket);
  out_response(socket, stdout);
  return 0;
}
