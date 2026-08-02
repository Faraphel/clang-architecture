pipeline {
    agent any

    options {
        skipDefaultCheckout(true)
    }

    stages {
        stage('Dependencies') {
            steps {
                sh '''
                # update sources
                apt-get update

                # build
                apt-get install -y cmake ninja-build build-essential

                # dependencies
                apt-get install -y clang libclang-dev llvm llvm-dev libcatch2-dev

                # document
                apt-get install -y doxygen

                # package
                apt-get install -y rpm
                '''
            }
        }

        stage('Checkout') {
            steps {
                checkout([
                    $class: 'GitSCM',
                    branches: scm.branches,
                    userRemoteConfigs: scm.userRemoteConfigs,
                    extensions: [
                        [
                            $class: 'SubmoduleOption',
                            recursiveSubmodules: true,
                            parentCredentials: true
                        ]
                    ]
                ])
            }
        }

        stage('Configure') {
            steps {
                sh '''
                cmake --preset release
                cmake --preset debug
                '''
            }
        }

        stage('Build') {
            steps {
                sh '''
                cmake --build --parallel --preset release
                cmake --build --parallel --preset debug
                '''
            }
        }

        stage('Document') {
            steps {
                sh '''
                doxygen
                '''
            }
        }

        stage('Test') {
            steps {
                sh '''
                ctest --preset debug --output-on-failure
                '''
            }
        }

        stage('Package') {
            steps {
                sh '''
                cpack --preset release
                '''
            }
        }
    }
}