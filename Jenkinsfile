pipeline {
    agent { dockerfile true }
    stages {
        stage('Build GCC') {
            steps {
                sh 'make clean'
                sh 'make -j4 CXX=g++'
            }
        }
        stage('Test GCC') {
            steps {
                sh '. ./asan_env && make -j4 CXX=g++ clean check'
            }
        }
        stage('Build Clang') {
            steps {
                sh 'make clean'
                sh 'make -j4 CXX=clang++'
            }
        }
        stage('Test Clang') {
            steps {
                sh '. ./asan_env && make -j4 CXX=clang++ clean check'
            }
        }
    }
}
