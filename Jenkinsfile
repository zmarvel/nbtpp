pipeline {
    agent { dockerfile true }
    stages {
        stage('Build GCC') {
            steps {
                sh 'cmake -S . -B build.gcc -G Ninja -DCMAKE_BUILD_TYPE=Debug'
                sh 'cmake --build build.gcc'
            }
        }
        stage('Test GCC') {
            steps {
                sh 'make -C test/data check'
                sh './build.gcc/test_nbt'
            }
        }
        stage('Build Clang') {
            steps {
                sh 'cmake -S . -B build.clang -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=/usr/bin/clang++'
                sh 'cmake --build build.clang'
            }
        }
        stage('Test Clang') {
            steps {
                sh './build.clang/test_nbt'
            }
        }
    }
}
