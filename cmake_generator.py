
# -O2 -fopenmp -DUSE_OMP -DNUM_THREADS=4 -std=c++17 -pthread

class CMakeGenerator:
    def __init__(self):
        self.template_file_name = 'CMakeListsTemplate.txt'
        self.template_flags_pholder = '%ADDITIONAL_FLAGS%'
        self.lists_backup = None

    def save_backup(self):
        with open('CMakeLists.txt', 'r') as inp:
            self.lists_backup = inp.read()

    def generate(self, use_o2=False, use_omp=False, omp_threads=None, cpp_version=17, use_pthread=True):
        assert(use_omp == False or (use_omp == True and (omp_threads is not None))) # If use_omp is set number of threads should be set too
        self.save_backup()

        flags = ''
        if use_o2:
            flags += '-O2 '
        if use_omp:
            flags += '-fopenmp '
            flags += '-DUSE_OMP '
            flags += '-DNUM_THREADS=' + str(omp_threads) + ' '
        if use_pthread:
            flags += '-pthread '
        flags += '--std=c++' + str(cpp_version)

        with open(self.template_file_name, 'r') as inp:
            result = inp.read().replace(self.template_flags_pholder, flags)
        with open('CMakeLists.txt', 'w') as out:
            out.write(result)
        

    def restore_backup(self):
        if self.lists_backup is None:
            return
        with open('CMakeLists.txt', 'w') as out:
            out.write(self.lists_backup)
