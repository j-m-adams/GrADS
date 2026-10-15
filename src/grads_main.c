/* Native Windows launcher. The engine itself lives in gradspy.dll. */

int gamain(int argc, char **argv);

int main(int argc, char **argv) {
  return gamain(argc, argv);
}
