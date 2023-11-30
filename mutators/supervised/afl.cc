#include "MutatorManager.h"
#include "json.hpp"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <string>

#include "afl-fuzz.h"

#define DATA_SIZE (100)

extern "C" {

typedef struct ysmut_afl_state {
  afl_state_t *afl;
  // any additional data here!
  ysmut::MutatorManager *manager;
  std::string program;
  std::unique_ptr<llvm::raw_string_ostream> oss;
} ysmut_afl_state_t;

ysmut_afl_state_t *afl_custom_init(afl_state_t *afl, unsigned int seed) {
  srand(seed);  // needed also by surgical_havoc_mutate()
  ysmut_afl_state_t *data = new ysmut_afl_state_t;
  if (!data) {
    perror("afl_custom_init alloc");
    return NULL;
  }
  data->afl = afl;

  data->manager = ysmut::MutatorManager::getInstance();
  data->manager->srand(seed);;
  data->oss.reset(new llvm::raw_string_ostream(data->program));
  data->manager->setOutStream(*(data->oss));
  return data;
}

size_t afl_custom_fuzz(ysmut_afl_state_t *data, uint8_t *buf, size_t buf_size,
                       u8 **out_buf, uint8_t *add_buf,
                       size_t add_buf_size,
                       size_t max_size) {

  size_t mutated_size = DATA_SIZE <= max_size ? DATA_SIZE : max_size;
  auto mutators = data->manager->getAllMutators();

  bool success = false;
  data->oss->flush();
  data->program.clear();
  std::shuffle(mutators.begin(), mutators.end(),
      data->manager->getRandomGenerator());
  data->manager->setSourceText(std::string((char *)buf, buf_size));
  for (auto &m : mutators) {
    data->manager->setMutator(m);
    if (data->manager->mutate()) {
      success = true;
      break;
    }
  }

  if (!success) {
    *out_buf = NULL;
    return 0;
  }

  data->oss->flush();
  *out_buf = (uint8_t *)&(data->program[0]);
  return data->program.size();
}

void afl_custom_deinit(ysmut_afl_state_t *data) {
  delete data;
}

}
