/*
Copyright (c) 2008-2015 Jesse Beder.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef SETTING_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define SETTING_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include <memory>
#include <utility>
#include <vector>

namespace YAML {
class SettingChangeBase;

template <typename T>
class Setting {
 public:
  Setting() : m_value() {}
  Setting(const T& value) : m_value() { set(value); }

  const T get() const { return m_value; }
  std::unique_ptr<SettingChangeBase> set(const T& value);
  void restore(const Setting<T>& oldSetting) { m_value = oldSetting.get(); }

 private:
  T m_value;
};

class SettingChangeBase {
 public:
  virtual ~SettingChangeBase() {}
  virtual void pop() = 0;
};

template <typename T>
class SettingChange : public SettingChangeBase {
 public:
  SettingChange(Setting<T>* pSetting)
      : m_pCurSetting(pSetting),
        m_oldSetting(*pSetting)  // copy old setting to save its state
  {}
  SettingChange(const SettingChange&) = delete;
  SettingChange(SettingChange&&) = delete;
  SettingChange& operator=(const SettingChange&) = delete;
  SettingChange& operator=(SettingChange&&) = delete;

  virtual void pop() { m_pCurSetting->restore(m_oldSetting); }

 private:
  Setting<T>* m_pCurSetting;
  Setting<T> m_oldSetting;
};

template <typename T>
inline std::unique_ptr<SettingChangeBase> Setting<T>::set(const T& value) {
  std::unique_ptr<SettingChangeBase> pChange(new SettingChange<T>(this));
  m_value = value;
  return pChange;
}

class SettingChanges {
 public:
  SettingChanges() : m_settingChanges{} {}
  SettingChanges(const SettingChanges&) = delete;
  SettingChanges& operator=(const SettingChanges&) = delete;
  ~SettingChanges() { clear(); }

  void clear() {
    restore();
    m_settingChanges.clear();
  }

  void restore() {
    for (setting_changes::const_iterator it = m_settingChanges.begin();
         it != m_settingChanges.end(); ++it)
      (*it)->pop();
  }

  void push(std::unique_ptr<SettingChangeBase> pSettingChange) {
    m_settingChanges.push_back(std::move(pSettingChange));
  }

  // like std::unique_ptr - assignment is transfer of ownership
  SettingChanges& operator=(SettingChanges&& rhs) {
    if (this == &rhs)
      return *this;

    clear();
    std::swap(m_settingChanges, rhs.m_settingChanges);

    return *this;
  }

 private:
  typedef std::vector<std::unique_ptr<SettingChangeBase>> setting_changes;
  setting_changes m_settingChanges;
};
}  // namespace YAML

#endif  // SETTING_H_62B23520_7C8E_11DE_8A39_0800200C9A66
