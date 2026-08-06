pipeline {
    agent {
        label 'debian'
    }

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
                apt-get install -y esbuild

                # dependencies
                apt-get install -y clang libclang-dev llvm llvm-dev libcatch2-dev

                # document
                apt-get install -y doxygen

                # analysis
                apt-get install -y python3.13 python3-pip python3-venv
                apt-get install -y clang-tools clang-tidy gcovr

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
                        sh '''
                        cmake --preset release
                        '''
                    }
                }

                stage('Configure Debug') {
                    steps {
                        sh '''
                        cmake --preset debug
                        '''
                    }
                }
            }
        }

        stage('Build') {
            parallel {
                stage('Build Release') {
                    steps {
                        sh '''
                        cmake --build --parallel --preset release
                        '''
                    }
                }

                stage('Build Debug') {
                    steps {
                        sh '''
                        cmake --build --parallel --preset debug
                        '''
                    }
                }
            }
        }

        stage('Post Build') {
            parallel {

                stage('Self-Run') {
                    steps {
                        sh '''
                        ./build/release/clang-architecture \
                            --extra-arg=-resource-dir=$(clang -print-resource-dir) \
                            -p ./build/debug/ \
                            --output ./build/debug/architecture.json \
                            $(find ./source/ -type f)
                        '''
                    }
                }

                stage('Document') {
                    steps {
                        sh '''
                        DOXYGEN_PROJECT_NUMBER=$GIT_COMMIT doxygen ./docs/Doxyfile
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

                        stage('SAST') {
                            steps {
                                sh '''
                                # create a virtual environment
                                python3 -m venv ./.venv

                                # install the environment
                                . ./.venv/bin/activate
                                pip install --upgrade pip
                                pip install codechecker

                                # run the analysis
                                CodeChecker analyze \
                                    --analyzers clangsa clang-tidy \
                                    --output ./build/debug/reports/codechecker/raw \
                                    --file ./source \
                                    -- \
                                    ./build/debug/compile_commands.json

                                # export as HTML
                                CodeChecker parse \
                                    --export sarif \
                                    --output ./build/debug/reports/codechecker/sarif/report.sarif \
                                    --file ./source \
                                    -- \
                                    ./build/debug/reports/codechecker/raw || true
                                '''
                            }
                        }

                        stage('Coverage') {
                            steps {
                                sh '''
                                mkdir -p ./build/debug/reports/coverage/

                                gcovr \
                                    --root ./ \
                                    --filter 'source/' \
                                    --txt-metric branch \
                                    --cobertura ./build/debug/reports/coverage/cobertura.xml \
                                    --print-summary \
                                    --sort uncovered-percent \
                                    --sort uncovered-number
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
                    build/release/*.zip,
                    build/debug/architecture.json
                '''.stripIndent().trim(),
                fingerprint: true
            )

            publishHTML([
                allowMissing: false,
                alwaysLinkToLastBuild: true,
                keepAll: true,
                reportDir: 'docs/build/html',
                reportFiles: 'index.html',
                reportName: 'Doxygen Documentation'
            ])

            recordIssues(
                enabledForFailure: true,
                tools: [
                    sarif(pattern: 'build/debug/reports/codechecker/sarif/report.sarif')
                ]
            )

            recordCoverage(
                enabledForFailure: true,
                sourceCodeRetention: 'MODIFIED',
                tools: [[
                    parser: 'COBERTURA',
                    pattern: 'build/debug/reports/coverage/cobertura.xml'
                ]]
            )
        }
    }
}
