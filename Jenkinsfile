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
                apt-get install -y python3.13 python3-pip python3-venv
                apt-get install -y clang-tools clang-tidy

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
                            $class: 'CloneOption',
                            shallow: true,
                            depth: 1,
                            noTags: true
                        ],
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
            parallel {
                stage('Configure Release') {
                    steps {
                        sh 'cmake --preset release'
                    }
                }

                stage('Configure Debug') {
                    steps {
                        sh 'cmake --preset debug'
                    }
                }
            }
        }

        stage('Build') {
            parallel {
                stage('Build Release') {
                    steps {
                        sh 'cmake --build --parallel --preset release'
                    }
                }

                stage('Build Debug') {
                    steps {
                        sh 'cmake --build --parallel --preset debug'
                    }
                }
            }
        }

        stage('Post Build') {
            parallel {

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

                stage('Analysis') {

                    stages {

                        stage('Analyze') {
                            steps {
                                sh '''
                                # create a virtual environment
                                python3 -m venv ./.venv

                                # install the environment
                                . ./.venv/bin/activate
                                pip install --upgrade pip
                                pip install codechecker

                                # run the analysis
                                ./.venv/bin/CodeChecker analyze \
                                    --analyzers clangsa clang-tidy \
                                    --output ./build/debug/reports \
                                    ./build/debug/compile_commands.json

                                # export as SARIF
                                ./.venv/bin/CodeChecker parse \
                                    --export sarif \
                                    --output ./build/debug/reports/report.sarif \
                                    ./build/debug/reports
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
                                            -Dsonar.externalIssuesReportPaths=build/debug/reports/report.sarif
                                        """
                                    }
                                }
                            }
                        }

                    }

                }
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
