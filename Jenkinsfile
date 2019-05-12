pipeline {
    agent any
    stages {
        stage('Build GCC') {
            steps {
                sh 'make clean'
                sh 'make CXX=g++'
                sh 'make CXX=g++ test'
            }
        }
        stage('Test GCC') {
            steps {
                sh './test_nbt'
            }
        }
        stage('Build Clang') {
            steps {
                sh 'make clean'
                sh 'make CXX=clang++'
                sh 'make CXX=clang++ test'
            }
        }
        stage('Test Clang') {
            steps {
                sh './test_nbt'
            }
        }
    }
}
