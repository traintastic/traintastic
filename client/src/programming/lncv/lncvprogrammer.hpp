/**
 * client/src/programming/lncv/lncvprogrammer.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef TRAINTASTIC_CLIENT_PROGRAMMING_LNCV_LNCVPROGRAMMER_HPP
#define TRAINTASTIC_CLIENT_PROGRAMMING_LNCV_LNCVPROGRAMMER_HPP

#include <QWidget>
#include <memory>
#include <unordered_map>
#include "../../network/objectptr.hpp"

class Connection;
class QStackedWidget;
class QStatusBar;
class QComboBox;
class QSpinBox;
class QCheckBox;
class QPushButton;
class QTableWidget;

class LNCVProgrammer final : public QWidget
{
  private:
    enum class State
    {
      Idle,
      WaitForStart,
      WaitForRead,
      WaitForWrite,
    };

    static constexpr int pageStart = 0;
    static constexpr int pageProgramming = 1;
    static constexpr int roleFilename = Qt::UserRole + 0;
    static constexpr int roleModuleId = Qt::UserRole + 1;
    static constexpr int showMessageTimeout = 3000;
    static constexpr int moduleIdMin = 0;
    static constexpr int moduleIdMax = 65535;
    static constexpr uint16_t broadcastAddress = 65535;
    static constexpr int lncvMin = 0;
    static constexpr int lncvMax = 655535;
    static constexpr int lncvValueMin = 0;
    static constexpr int lncvValueMax = 655535;
    static constexpr int columnLNCV = 0;
    static constexpr int columnValue = 1;
    static constexpr int columnDescription = 2;

    std::shared_ptr<Connection> m_connection;
    int m_requestId;
    ObjectPtr m_object;
    QStackedWidget* m_pages;
    QStatusBar* m_statusBar;
    QComboBox* m_interface;
    QComboBox* m_module;
    QSpinBox* m_otherModule;
    QSpinBox* m_address;
    QCheckBox* m_broadcastAddress;
    QTableWidget* m_lncvs;
    QPushButton* m_start;
    QPushButton* m_read;
    QPushButton* m_write;
    QPushButton* m_stop;
    int m_lastAddress = 1;
    State m_state = State::Idle;
    std::unordered_map<int, int> m_lncvToRow;

    void loadInterfaces();
    void loadModules();
    void loadLNCVs();
    void moduleChanged();
    void useBroadcastAddressChanged();
    void updateStartEnabled();
    void updateReadWriteEnabled();
    void reset();
    int getSelectedLNCV() const;
    int getSelectedLNCVValue() const;
    void setState(State value);

  public:
    explicit LNCVProgrammer(std::shared_ptr<Connection>, QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~LNCVProgrammer() final;
};

#endif
