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

                # analysis
                apt-get install -y clang-tools analyze-build clang-tidy

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

                script {
                    env.GIT_COMMIT = sh(
                        script: 'git rev-parse HEAD',
                        returnStdout: true
                    ).trim()
                    env.GIT_REPO_URL = sh(
                        script: 'git config --get remote.origin.url',
                        returnStdout: true
                    ).trim()
                    env.GIT_REPO_NAME = sh(
                         script: "basename -s .git '${env.GIT_REPO_URL}'",
                         returnStdout: true
                    ).trim()

                    echo "Repository URL: ${env.GIT_REPO_URL}"
                    echo "Repository Name: ${env.GIT_REPO_NAME}"
                    echo "Repository Commit: ${env.GIT_COMMIT}"
                }
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
                DOXYGEN_PROJECT_NUMBER=$GIT_COMMIT doxygen Doxyfile
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

        stage('Clang Static Analyzer') {
            steps {
                sh '''
                mkdir -p ./build/debug/reports/clangsa/

                analyze-build \
                    --cdb build/debug/compile_commands.json \
                    --stats \
                    --plist \
                    --exclude external \
                    --exclude tests \
                    --output ./build/debug/reports/clangsa
                '''
            }
        }

        stage('Clang-Tidy') {
            steps {
                sh '''
                mkdir -p ./build/debug/reports/clangtidy/

                clang-tidy \
                    -p build/debug \
                    --header-filter=./source/* \
                    source/* \
                    tests/* \
                > ./build/debug/reports/clangtidy/report.txt
                '''
            }
        }

        stage('Report') {
            steps {
                script {
                    def scannerHome = tool 'SonarScanner'

                    withSonarQubeEnv('SonarQube') {
                        sh """
                        ${scannerHome}/bin/sonar-scanner \
                            -Dsonar.projectKey=${GIT_REPO_NAME} \
                            -Dsonar.projectName=Clang-Architecture \
                            -Dsonar.projectVersion=${GIT_COMMIT} \
                            -Dsonar.sources=source,viewer/javascript \
                            -Dsonar.exclusions=viewer/dist/**/* \
                            -Dsonar.test.inclusions=tests/**/* \
                            -Dsonar.cxx.jsonCompilationDatabase=build/debug/compile_commands.json \
                            -Dsonar.cxx.file.suffixes=.cpp,.c,.hpp,.h \
                            -Dsonar.cxx.clangsa.reportPaths=build/debug/reports/clangsa/ \
                            -Dsonar.cxx.clangtidy.reportPaths=./build/debug/reports/clangtidy/
                        """
                    }
                }
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

    post {
        success {
            archiveArtifacts(
                artifacts: '''
                    build/release/*.deb,
                    build/release/*.rpm,
                    build/release/*.tar.gz,
                    build/release/*.zip
                '''.stripIndent().trim(),
                fingerprint: true
            )

            publishHTML([
                allowMissing: false,
                alwaysLinkToLastBuild: true,
                keepAll: true,
                reportDir: 'docs/html',
                reportFiles: 'index.html',
                reportName: 'Doxygen Documentation'
            ])
        }
    }
}
