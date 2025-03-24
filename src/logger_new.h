#pragma once

#include <fstream>
#include <string>
#include <mutex>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

class Logger {
public:
    enum class Level {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };
    
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }
    
    void initialize(const std::string& logFile) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Crear directorio para logs si no existe
        fs::path logPath = fs::path(logFile).parent_path();
        if (!logPath.empty() && !fs::exists(logPath)) {
            try {
                fs::create_directories(logPath);
            } catch (const std::exception& e) {
                std::cerr << "Error al crear directorio de logs: " << e.what() << std::endl;
            }
        }
        
        m_logFile.open(logFile, std::ios::out | std::ios::app);
        if (!m_logFile.is_open()) {
            std::cerr << "Error al abrir archivo de log: " << logFile << std::endl;
        }
        
        log(Level::INFO, "Sistema de logs inicializado");
    }
    
    void log(Level level, const std::string& message) {
        if (!m_logFile.is_open()) {
            return;
        }
        
        std::lock_guard<std::mutex> lock(m_mutex);
        
        std::string levelStr;
        switch (level) {
            case Level::DEBUG:   levelStr = "DEBUG"; break;
            case Level::INFO:    levelStr = "INFO"; break;
            case Level::WARNING: levelStr = "WARNING"; break;
            case Level::ERROR:   levelStr = "ERROR"; break;
            default:             levelStr = "UNKNOWN"; break;
        }
        
        m_logFile << "[" << levelStr << "] " << message << std::endl;
        m_logFile.flush();
    }
    
    ~Logger() {
        if (m_logFile.is_open()) {
            log(Level::INFO, "Sistema de logs finalizado");
            m_logFile.close();
        }
    }
    
private:
    Logger() = default;
    
    std::ofstream m_logFile;
    std::mutex m_mutex;
};

// Macros para facilitar el uso del logger
#define LOG_DEBUG(msg) Logger::getInstance().log(Logger::Level::DEBUG, msg)
#define LOG_INFO(msg) Logger::getInstance().log(Logger::Level::INFO, msg)
#define LOG_WARNING(msg) Logger::getInstance().log(Logger::Level::WARNING, msg)
#define LOG_ERROR(msg) Logger::getInstance().log(Logger::Level::ERROR, msg) 