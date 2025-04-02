
internal void logprint(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  cstring string = cstring_format_va(fmt, args);

  fprintf(stderr, string);

  va_end(args);

  cstring_free(string);
}
