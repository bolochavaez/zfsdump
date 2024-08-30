/*
 * This program scans the ZFS module parameters directory and prints
 * the parameters in JSON format.
 * handles errors (semi) gracefully.
 */

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int filter_hidden(const struct dirent *entry) {
  if (entry->d_name[0] == '.')
    return 0;
  else
    return 1;
}

void read_all(char *dest, FILE *src) {
  int read_size = 0;
  char data_buffer[100];
  strcpy(dest, "");
  do {
    read_size = fread(data_buffer, 1, sizeof(data_buffer) - 1, src);
    data_buffer[read_size] = '\0';
    strcat(dest, data_buffer);
  } while (read_size > 0);
}

void grab_parameter(char *data_value, char *location) {
  FILE *parameter_info = fopen(location, "rt");
  if (!parameter_info) {
    printf("failed opening file");
    perror("fopen: ");
    exit(1);
  }
  read_all(data_value, parameter_info);
  if (data_value[strlen(data_value) - 1] == '\n')
    data_value[strlen(data_value) - 1] = '\0';
  fclose(parameter_info);
}

int main() {
  struct dirent **zfs_param_files;
  int number_of_files;
  char pathname_buffer[100];
  char data_value[100];
  char path_root[] = "/sys/module/zfs/parameters/";

  number_of_files = scandir("/sys/module/zfs/parameters", &zfs_param_files,
                            filter_hidden, alphasort);
  if (number_of_files == -1) {
    perror("scandir");
    return 1;
  }

  printf("{\n");
  for (int i = 0; i < number_of_files; i++) {
    strcpy(pathname_buffer, path_root);
    strcat(pathname_buffer, zfs_param_files[i]->d_name);
    grab_parameter(data_value, pathname_buffer);
    printf("\"%s\":\"%s\"", zfs_param_files[i]->d_name, data_value);
    printf("%s\n", (i < number_of_files - 1) ? "," : "");
    free(zfs_param_files[i]);
  }
  free(zfs_param_files);
  printf("}\n");
}
