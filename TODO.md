# Medusa
Medusa is a C++ / Python TCP socket application designed to test my knowledge of the languages

## Uncompleted tasks
- [x] Deprecate the call to subprocess.run ~1d #optimization @Narr0wB 27-03-2022 - 28-03-2022
  - Description: Deprecate the call to subprocess.run in the python side of the code in favour of a child cmd process
  - Solution: Kept the call to subprocess.run and added a currentDirectory system to the python side of code  

- [x] Add support for file tranfer ~3d/5d #functionality @Narr0wB 27-03-2022 - 02-04-2022
  - Description: If point 1 was completed, add support for moderate size file transfer (1-2GB max)

- [x] Add a terminal-like nature of the client console ~1d/2d  #functionality @Narr0wB 31-03-2022 - 02-04-2022
  - Description: Have the current dir shown after any command
 
- [ ] Implement a gui library for the client ~7d/14d #design @Narr0wB 27-03-2022
  - Description: Implement a gui in the client code so that it could show the video feedback it receives (ImGui libraries or others..)
