#pragma once

namespace vkx
{
    struct Event
    {
        virtual ~Event() = default;
    };

    template <class T>
    class Observer
    {
    public:
        virtual ~Observer() = default;

        virtual void update(T const &event) = 0;
    };

    template <class T, class K>
    class Subject
    {
    public:
        virtual ~Subject() = default;

        void attach(T *observer)
        {
            observers.push_back(observer);
        }

        void detach(T *observer)
        {
            observers.erase(std::ranges::remove(observers, observer), observers.begin);
        }

    protected:
        void notify(K const &event)
        {
            std::ranges::for_each(observers, [&event](auto const &observer)
            {
                observer->update(event);
            });
        }

    private:
        std::vector<T*> observers;
    };

    struct MouseEvent : public Event
    {
        MouseEvent() = default;

        MouseEvent(glm::vec2 const &position, glm::vec2 const &relative)
            : position(position), relative(relative) {}

        // Current position
        glm::vec2 position;
        // Relative to last position
        glm::vec2 relative;
    };

    class MouseObserver : public Observer<MouseEvent>
    {};

    class MouseSubject : public Subject<MouseObserver, MouseEvent>
    {};

    struct KeyboardEvent : public Event
    {
        KeyboardEvent() = default;

        KeyboardEvent(int key, int scancode, int action, int mods)
            : key(key), scancode(scancode), action(action), mods(mods) {}

        int key;
        int scancode;
        int action;
        int mods;
    };

    class KeyboardObserver : public Observer<KeyboardEvent>
    {};

    class KeyboardSubject : public Subject<KeyboardObserver, KeyboardEvent>
    {};

    struct FramebufferResizedEvent : public Event
    {
        FramebufferResizedEvent() = default;

        FramebufferResizedEvent(int width, int height)
            : width(width), height(height) {}

        int width;
        int height;
    };

    class FramebufferResizedObserver : public Observer<FramebufferResizedEvent>
    {};

    class FramebufferResizedSubject : public Subject<FramebufferResizedObserver, FramebufferResizedEvent>
    {};
}