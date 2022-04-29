#pragma once

template<class ValueType> struct AnyInPlaceTypeT {
    explicit AnyInPlaceTypeT() = default;
};

/**
 * @brief
 * 
 */
class FAny
{
private:
    /**
     * @brief 
     */
    enum class EManagerOperator : uint8
    {
        Clone,
        Move,
        Destroy,
        GetDisplayInfo
    };

    /**
     * @brief 
     */
    struct FManagerOutput
    {
        FString DisplayInfo;
    };

    template<class _ValueType>
    class TManager
    {
    public:
	    template<class ValueType>
        static void Init(FAny* Any, ValueType&& Value)
        {
            Any->Storage = new _ValueType{ std::forward<ValueType>(Value) };
        }

        template<class... Args>
        static void Init(FAny* Any, Args&&... Args_)
        {
            Any->Storage = new _ValueType{ std::forward<Args>(Args_)... };
        }
        
        /**
         * Manage a TAny instance depending on a specific operator
         * 
         * @param Op    The operation to execute, either modify the Left instance or execute a query
         * @param Any  The instance that called the function and can be changed
         * @warning     The `Left` TAny instance constness is ignored during clone, move and destroy operation
         * @param Other The instance used to either query (ie : operator==) or change the Left instance (ie: copy)
         * @warning     The `Right` TAny instance constness is ignored during move
         */
        static FManagerOutput Manage(EManagerOperator Op, const FAny* Any, const FAny* Other = nullptr)
        {
            // Remove const from Left, should be used only by op that implicitly imply that a non const object was actually passed
            FAny* ConstlessLeft = const_cast<FAny*>(Any);
            FAny* ConstlessRight = const_cast<FAny*>(Other);

            FManagerOutput Output;   
            switch (Op)
            {
            case EManagerOperator::Clone:
                ConstlessLeft->Storage = new _ValueType{ *static_cast<_ValueType*>(Other->Storage) };
                ConstlessLeft->Manager = Other->Manager;
                break;
            case EManagerOperator::Move:
                ConstlessLeft->Storage = new _ValueType{ *static_cast<_ValueType*>(Other->Storage) };
                ConstlessLeft->Manager = Other->Manager;
                ConstlessRight->Manager = nullptr;
                break;
            case EManagerOperator::Destroy:
                delete static_cast<_ValueType*>(ConstlessLeft->Storage);
                ConstlessLeft->Manager = nullptr;
                break;
            case EManagerOperator::GetDisplayInfo:
                //Output.DisplayInfo = Any->Storage ? AnyDebug::TAnyDebugger<_ValueType>::GetDisplayInfo(static_cast<_ValueType*>(Any->Storage)) : "Null";
                break;
            }   
            return Output;
        }
    };

    void *Storage;
    FManagerOutput (*Manager) (EManagerOperator Operator, const FAny* Any, const FAny* Other);
    
public:
	FAny() noexcept : Storage{ nullptr }, Manager { nullptr } {}

	~FAny()
    {
        Reset();
    }

    // Clone constructor
    FAny(const FAny& Other)
    {
        if (Other.HasValue())
            (void) Other.Manage(EManagerOperator::Clone, this, &Other);
    }

    void swap(FAny& Other) noexcept
    {
        using std::swap;
        swap(this->Storage, Other.Storage);
        swap(this->Manager, Other.Manager);
    }

    friend void swap(FAny& Left, FAny& Right) noexcept
    {
        Left.swap(Right);
    }

    // Clone assignment
    FAny& operator=(const FAny& Other)
    {
        FAny{ Other }.swap(*this);
        return *this;
    }

    // Move constructor
    FAny(FAny&& Other) noexcept
    {
        *this = std::move(Other);
    }

    // Move assignment
    FAny& operator=(FAny&& Other) noexcept
    {
        if (!Other.HasValue())
            Reset();
        else if (&Other != this) {
            Reset();
            (void) Other.Manage(EManagerOperator::Move, this, &Other);
        }
        return *this;
    }

    // Copy constructor for stored type
    template<class ValueType,
        typename _ValueType = std::enable_if_t<!std::is_same<std::decay_t<ValueType>, FAny>::value, std::decay_t<ValueType>>,
        typename Mgr = TManager<_ValueType>,
        typename = std::enable_if_t<std::is_copy_constructible<_ValueType>::value>>
        FAny(ValueType&& Value) : Manager { &Mgr::Manage }
    {
        TManager<_ValueType>::Init(this, std::forward<ValueType>(Value));
    }

    // Store a copy of Other
    template<class ValueType>
    std::enable_if_t<!std::is_same<std::decay_t<ValueType>, FAny>::value && std::is_copy_constructible<std::decay_t<ValueType>>::value, FAny&>
        operator=(ValueType&& Value)
    {
        *this = FAny{ std::forward<ValueType>(Value) };
        return *this;
    }

    template<class ValueType, class... Args, typename _ValueType = std::decay_t<ValueType>>
    explicit FAny(AnyInPlaceTypeT<ValueType>, Args&&... args) : Manager{ &TManager<_ValueType>::Manage }
    {
        static_assert(std::is_constructible<_ValueType, Args...>::value, "ill formed");
        static_assert(std::is_copy_constructible<_ValueType>::value, "ill formed");
        TManager<_ValueType>::Init(this, std::forward<Args>(args)...);
    }

    template<class ValueType, class U, class... Args, typename _ValueType = std::decay_t<ValueType>>
    explicit FAny(AnyInPlaceTypeT<ValueType>, std::initializer_list<U> il, Args&&... args) : Manager{ &TManager<_ValueType>::Manage }
    {
        static_assert(std::is_constructible<_ValueType, std::initializer_list<U>&, Args...>::value, "ill formed");
        static_assert(std::is_copy_constructible<_ValueType>::value, "ill formed");
        TManager<_ValueType>::Init(this, il, std::forward<Args>(args)...);
    }
    
    FManagerOutput Manage(EManagerOperator Op, const FAny* Left, const FAny* Right = nullptr) const
    {
        return Manager(Op, Left, Right);
    }

    /** If not empty, destroys the contained object and reset state. */
    void Reset() noexcept
    {
        if (HasValue())
        {
            (void) Manage(EManagerOperator::Destroy, this);
        }
    }

    template<class ValueType, typename _ValueType = std::decay_t<ValueType>>
    bool CanCastTo() const noexcept
    {
        return Manager == &TManager<_ValueType>::Manage;
    }

    bool HasSameType(const FAny& Other) const noexcept
    {
        return Manager == Other.Manager;
    }

    bool HasValue() const noexcept
    {
        return Manager != nullptr;
    }

    FString GetDisplayInfo() const noexcept
    {
        return HasValue() ? Manage(EManagerOperator::GetDisplayInfo, this).DisplayInfo : "";
    }
   
    template <class T>
    friend T* AnyCastImpl(const FAny* Any) noexcept;
};

template<class ValueType, class... Args>
FAny MakeAny(Args&&... args)
{
    return FAny{ AnyInPlaceTypeT<ValueType>{}, std::forward<Args>(args)... };
}

template<class ValueType, class U, class... Args>
FAny MakeAny(std::initializer_list<U> il, Args&&... args)
{
    return FAny{AnyInPlaceTypeT<ValueType>{}, il, std::forward<Args>(args)...};
}

struct FBadAnyCast {
    const char* what() noexcept
    {
        return "Bad AnyCast";
    }
};

template<class T>
T* AnyCastImpl(const FAny* Any) noexcept
{
    using U = std::remove_cv_t<std::remove_reference_t<T>>;
    
    // Stored type is decayed, if U is not decayed, it's sure it will not be stored
    if (!std::is_same<std::decay_t<U>, U>::value)
        return nullptr;

    // If not copy constructible, it's sure it will not be stored
    if (!std::is_copy_constructible<U>::value)
        return nullptr;

    // If the manager don't match, it's sure it will not be stored
    if (Any->Manager == &FAny::TManager<U>::Manage)
        return static_cast<T*>(const_cast<void*>(Any->Storage));

    return nullptr;
}

template<class T>
const T* AnyCast(const FAny* Any) noexcept
{
    if (std::is_object<T>::value && Any)
    {
        return AnyCastImpl<T>(Any);
    }

    return nullptr;
}

template<class T>
T* AnyCast(FAny* Any) noexcept
{
    if (std::is_object<T>::value && Any)
    {
        return AnyCastImpl<T>(Any);
    }

    return nullptr;
}

template<class T>
T AnyCast(const FAny& Any)
{
    using U = std::remove_cv_t<std::remove_reference_t<T>>;
    static_assert(std::is_constructible<T, const U&>::value, "ill formed cast");
    
    auto p = AnyCast<U>(&Any);
    if (p)
        return static_cast<T>(*p);

    throw FBadAnyCast{};
}

template<class T>
T AnyCast(FAny& Any)
{
    using U = std::remove_cv_t<std::remove_reference_t<T>>;
    static_assert(std::is_constructible<T, U&>::value, "ill formed cast");

    auto p = AnyCast<U>(&Any);
    if (p)
        return static_cast<T>(*p);

    throw FBadAnyCast{};
}

template<class T>
T AnyCast(FAny&& Any)
{
    using U = std::remove_cv_t<std::remove_reference_t<T>>;
    static_assert(std::is_constructible<T, U>::value, "ill formed cast");

    auto p = AnyCast<U>(&Any);
    if (p)
        return static_cast<T>(std::move(*p));

    throw FBadAnyCast{};
}
