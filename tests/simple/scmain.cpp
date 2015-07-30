#include <tlm_router_test.h>
#include <vector>
#include <getopt.h>

#ifndef SEND_MAX
#define SEND_MAX 100
#endif

#ifndef NUM_TILES
#define NUM_TILES 1
#endif

#ifndef BUCKET_SIZE
#define BUCKET_SIZE 1
#endif

#ifndef WAIT_DONE
#define WAIT_DONE false
#endif

#ifndef SOURCE_TILE
#define SOURCE_TILE -1
#endif

#ifndef TARGET_TILE
#define TARGET_TILE -1
#endif

#ifndef MSGID
#define MSGID "test"
#endif

int sc_main(int argc, char *argv[]) {
  unsigned int ntiles = NUM_TILES;
  int source_tile = SOURCE_TILE;
  int target_tile = TARGET_TILE;
  unsigned int msgs = SEND_MAX;
  unsigned int bucket_size = BUCKET_SIZE;
  bool wait_done = WAIT_DONE;

  {
    int c;

    while (1) {
      static struct option long_options[] = {
          {"help", no_argument, 0, 'h'},
          {"ntiles", required_argument, 0, 'n'},
          {"source", required_argument, 0, 's'},
          {"target", required_argument, 0, 't'},
          {"msgs", required_argument, 0, 'm'},
          {"bucket", required_argument, 0, 'b'},
          {"wait", no_argument, 0, 'w'},
          {"nowait", no_argument, 0, '!'},
          {0, 0, 0, 0}};
      /* getopt_long stores the option index here. */
      int option_index = 0;

      c = getopt_long(argc, argv, "hn:s:t:m:b:w!", long_options, &option_index);

      /* Detect the end of the options. */
      if (c == -1)
        break;

      switch (c) {
      case 0:
        /* If this option set a flag, do nothing else now. */
        if (long_options[option_index].flag != 0)
          break;
        printf("option %s", long_options[option_index].name);
        if (optarg)
          printf(" with arg %s", optarg);
        printf("\n");
        break;

      case 'h':
        puts("Help\n");
        break;

      case 'n': // ntiles
        ntiles = atoi(optarg);
        break;

      case 's': // source
        source_tile = atoi(optarg);
        break;

      case 't': // target
        target_tile = atoi(optarg);
        break;

      case 'm': // msgs
        msgs = atoi(optarg);
        break;

      case 'b': // bucket
        bucket_size = atoi(optarg);
        break;

      case 'w': // wait
        wait_done = true;
        break;

      case '!': // no wait
        wait_done = false;
        break;

      case '?':
        /* getopt_long already printed an error message. */
        exit(1);
        break;

      default:
        abort();
      }
    }

    /* Print any remaining command line arguments (not options). */
    if (optind < argc) {
    }
  }

  router_table tableOfRouts;
  std::vector<init_dummy *> free_init_dummy;
  std::vector<target_dummy *> free_target_dummy;
  std::vector<router_test *> free_router_test;

  unsigned int cols = ceil(sqrt(ntiles));
  unsigned int rows = ceil(((double)ntiles) / cols);
  unsigned int ntiles_ext = rows * cols;

  fprintf(stderr, "Tiles: %u (%ux%u)\n", ntiles_ext, rows, cols);
  fprintf(stderr, "Tests: %u\n", ntiles);
  fprintf(stderr, "Source: %d\n", source_tile);
  fprintf(stderr, "Target: %d\n", target_tile);
  fprintf(stderr, "Messages: %u [%u]\n", msgs, bucket_size);
  if (wait_done)
    fprintf(stderr, "Wait done\n");
  else
    fprintf(stderr, "No wait done\n");

  router ***routers;
  routers = (router ***)malloc(cols * sizeof(router **));
  for (unsigned int i = 0; i < rows; i++) {
    routers[i] = (router **)malloc(cols * sizeof(router *));
  }

  // Creating Tiles
  {
    char comp_name[20];
    unsigned int i = 0;
    while (i < ntiles) {
      unsigned int r, c;
      r = i / cols;
      c = i % cols;

      // Creating Test module
      snprintf(comp_name, 20, "test_%03u", i);
      router_test *test =
          new router_test(comp_name, i, ntiles, source_tile, target_tile, msgs,
                          bucket_size, wait_done);
      free_router_test.push_back(test);

      // Creating Router
      snprintf(comp_name, 20, "router_%03u", i);
      router *rt = new router(comp_name);
      rt->setXY(r, c);

      // Linking Test and Router
      rt->LOCAL_init_socket.bind(test->tsocket);
      test->isocket.bind(rt->LOCAL_target_socket);

      i++;

      // Adding in Table of Routes
      tableOfRouts.newEntry(r, c, 1 << 16);

      routers[r][c] = rt;
    }

    while (i < ntiles_ext) {
      unsigned int r, c;
      r = i / cols;
      c = i % cols;
      // Creating Dummy Tile

      // Creating Router
      snprintf(comp_name, 20, "router_%03u", i);
      router *rt = new router(comp_name);
      rt->setXY(r, c);

      // Creating Dummy Init
      snprintf(comp_name, 20, "dinit_%03u", i);
      init_dummy *dummy_init = new init_dummy(comp_name);
      free_init_dummy.push_back(dummy_init);

      // Creating Dummy Target
      snprintf(comp_name, 20, "dtarget_%03u", i);
      target_dummy *dummy_target = new target_dummy(comp_name);
      free_target_dummy.push_back(dummy_target);

      // Linking Test and Router
      rt->LOCAL_init_socket.bind(dummy_target->socket);
      dummy_init->socket.bind(rt->LOCAL_target_socket);

      i++;
      routers[r][c] = rt;
    }
  }

  // Linking Tiles at NS
  // North / South
  {
    char comp_name[20];
    unsigned int r = rows - 1;
    unsigned int c = 0;
    for (c = 0; c < cols; c++) {
      // Binding Dummy North
      snprintf(comp_name, 20, "Ndinit_%03u_%03u", r, c);
      init_dummy *dummy_init = new init_dummy(comp_name);
      free_init_dummy.push_back(dummy_init);

      snprintf(comp_name, 20, "Ndtarget_%03u_%03u", r, c);
      target_dummy *dummy_target = new target_dummy(comp_name);
      free_target_dummy.push_back(dummy_target);

      routers[r][c]->N_init_socket.bind(dummy_target->socket);
      dummy_init->socket.bind(routers[r][c]->N_target_socket);
    }
    while (r) {
      r--;

      for (c = 0; c < cols; c++) {
        // Binding Noth
        routers[r][c]->N_init_socket.bind(routers[r + 1][c]->S_target_socket);

        // Binding South
        routers[r + 1][c]->S_init_socket.bind(routers[r][c]->N_target_socket);
      }
    }
    for (c = 0; c < cols; c++) {
      // Binding Dummy South
      snprintf(comp_name, 20, "Sdinit_%03u_%03u", r, c);
      init_dummy *dummy_init = new init_dummy(comp_name);
      free_init_dummy.push_back(dummy_init);

      snprintf(comp_name, 20, "Sdtarget_%03u_%03u", r, c);
      target_dummy *dummy_target = new target_dummy(comp_name);
      free_target_dummy.push_back(dummy_target);

      routers[r][c]->S_init_socket.bind(dummy_target->socket);
      dummy_init->socket.bind(routers[r][c]->S_target_socket);
    }
  }

  // Linking Tiles at WE
  // West / East
  {
    char comp_name[20];
    unsigned int r = 0;
    unsigned int c = 0;

    for (r = 0; r < rows; r++) {
      c = cols - 1;
      init_dummy *dummy_init;
      target_dummy *dummy_target;

      // Binding Dummy East
      snprintf(comp_name, 20, "Edinit_%03u_%03u", r, c);
      dummy_init = new init_dummy(comp_name);
      free_init_dummy.push_back(dummy_init);

      snprintf(comp_name, 20, "Edtarget_%03u_%03u", r, c);
      dummy_target = new target_dummy(comp_name);
      free_target_dummy.push_back(dummy_target);

      routers[r][c]->E_init_socket.bind(dummy_target->socket);
      dummy_init->socket.bind(routers[r][c]->E_target_socket);

      while (c) {
        c--;

        // Binding East
        routers[r][c]->E_init_socket.bind(routers[r][c + 1]->W_target_socket);

        // Binding West
        routers[r][c + 1]->W_init_socket.bind(routers[r][c]->E_target_socket);
      }

      // Binding Dummy West
      snprintf(comp_name, 20, "Wdinit_%03u_%03u", r, c);
      dummy_init = new init_dummy(comp_name);

      snprintf(comp_name, 20, "Wdtarget_%03u_%03u", r, c);
      dummy_target = new target_dummy(comp_name);
      free_target_dummy.push_back(dummy_target);

      routers[r][c]->W_init_socket.bind(dummy_target->socket);
      dummy_init->socket.bind(routers[r][c]->W_target_socket);
    }
  }

  // Sync Table of Routes
  for (unsigned int r = 0; r < rows; r++)
    for (unsigned int c = 0; c < cols; c++)
      tableOfRouts.appendTo(routers[r][c]->tableOfRouts);

  tableOfRouts.print();

  // Simulate
  try {
    SC_REPORT_INFO(MSGID, "Starting kernal");
#ifdef TEST_TIME
    sc_start(TEST_TIME, SC_NS, SC_RUN_TO_TIME);
#else
    sc_start();
#endif
  } catch (std::exception &e) {
    SC_REPORT_WARNING(MSGID, (string("Caught exception ") + e.what()).c_str());
  } catch (...) {
    SC_REPORT_ERROR(MSGID, "Caught exception during simulation.");
  } // endtry
  if (not sc_end_of_simulation_invoked()) {
    SC_REPORT_INFO(MSGID,
                   "ERROR: Simulation stopped without explicit sc_stop()");
    sc_stop();
    exit(1);
  } // endif
  SC_REPORT_INFO(MSGID, "The end.");

  while (!free_init_dummy.empty()) {
    delete (free_init_dummy.back());
    free_init_dummy.pop_back();
  }

  while (!free_target_dummy.empty()) {
    delete (free_target_dummy.back());
    free_target_dummy.pop_back();
  }

  while (!free_router_test.empty()) {
    delete (free_router_test.back());
    free_router_test.pop_back();
  }

  for (unsigned int r = 0; r < rows; r++) {
    for (unsigned int c = 0; c < cols; c++)
      delete (routers[r][c]);
    free(routers[r]);
  }
  free(routers);

  return (0);
}
